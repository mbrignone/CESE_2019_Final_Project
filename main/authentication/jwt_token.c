#include <jwt_token.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <mbedtls/pk.h>
#include <mbedtls/error.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <esp_wifi.h>
#include <esp_err.h>

#include "base64_url.h"


/**
 * Return a string representation of an mbedtls error code
 */
static char* mbedtlsError(int errnum) {
    static char buffer[200];
    mbedtls_strerror(errnum, buffer, sizeof(buffer));
    return buffer;
} // mbedtlsError


/**
 * Create a JWT token for GCP.
 * For full details, perform a Google search on JWT.  However, in summary, we build two strings.  One that represents the
 * header and one that represents the payload.  Both are JSON and are as described in the GCP and JWT documentation.  Next
 * we base64url encode both strings.  Note that is distinct from normal/simple base64 encoding.  Once we have a string for
 * the base64url encoding of both header and payload, we concatenate both strings together separated by a ".".   This resulting
 * string is then signed using RSASSA which basically produces an SHA256 message digest that is then signed.  The resulting
 * binary is then itself converted into base64url and concatenated with the previously built base64url combined header and
 * payload and that is our resulting JWT token.
 * @param projectId The GCP project.
 * @param privateKey The PEM or DER of the private key.
 * @param privateKeySize The size in bytes of the private key.
 * @returns A JWT token for transmission to GCP.
 */
jwt_token_t createGCPJWT(const char* projectId, const uint8_t* privateKey, size_t privateKeySize) {
    
    jwt_token_t current_token;
    
    char base64Header[100];
    const char header[] = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
    base64url_encode(
        (unsigned char *)header,   // Data to encode.
        strlen(header),            // Length of data to encode.
        base64Header);             // Base64 encoded data.

    time_t now;
    time(&now);
    uint32_t iat = now;                 // Set the time now.
    uint32_t exp = iat + TOKEN_PERIOD;  // Set the expiry time.

    current_token.exp_time = exp;

    char payload[100];
    sprintf(payload, "{\"iat\":%d,\"exp\":%d,\"aud\":\"%s\"}", iat, exp, projectId);

    char base64Payload[100];
    base64url_encode(
        (unsigned char *)payload,  // Data to encode.
        strlen(payload),           // Length of data to encode.
        base64Payload);            // Base64 encoded data.

    uint8_t headerAndPayload[300];
    sprintf((char*)headerAndPayload, "%s.%s", base64Header, base64Payload);

    // At this point we have created the header and payload parts, converted both to base64 and concatenated them
    // together as a single string.  Now we need to sign them using RSASSA

    mbedtls_pk_context pk_context;
    mbedtls_pk_init(&pk_context);
    int rc = mbedtls_pk_parse_key(&pk_context, privateKey, privateKeySize, NULL, 0);
    if (rc != 0) {
        printf("\n\n\nFailed to mbedtls_pk_parse_key: %d (-0x%x): %s\n\n\n", rc, -rc, mbedtlsError(rc));
        current_token.exp_time = 0;
        current_token.token = NULL;
        return current_token;
    }

    uint8_t oBuf[800];

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    const char* pers="MyEntropy";
    mbedtls_ctr_drbg_seed(
        &ctr_drbg,
        mbedtls_entropy_func,
        &entropy,
        (const unsigned char*)pers,
        strlen(pers));
    
    uint8_t digest[32];
    rc = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), headerAndPayload, strlen((char*)headerAndPayload), digest);
    if (rc != 0) {
        printf("Failed to mbedtls_md: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
        current_token.exp_time = 0;
        current_token.token = NULL;
        return current_token;
    }

    size_t retSize;
    rc = mbedtls_pk_sign(&pk_context, MBEDTLS_MD_SHA256, digest, sizeof(digest), oBuf, &retSize, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (rc != 0) {
        printf("Failed to mbedtls_pk_sign: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
        current_token.exp_time = 0;
        current_token.token = NULL;
        return current_token;
    }


    char base64Signature[600];
    base64url_encode((unsigned char *)oBuf, retSize, base64Signature);

    char* retData = (char*)malloc(strlen((char*)headerAndPayload) + 1 + strlen((char*)base64Signature) + 1);
    sprintf(retData, "%s.%s", headerAndPayload, base64Signature);

    current_token.token = retData;

    mbedtls_pk_free(&pk_context);

    return current_token;
}
