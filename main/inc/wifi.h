/* ===== [wifi.h] =====
 * Copyright Matias Brignone <mnbrignone@gmail.com>
 * All rights reserved.
 *
 * Version: 0.1.0
 * Creation Date: 2019
 */


/* ===== Avoid multiple inclusion ===== */
#ifndef __WIFI_H__
#define __WIFI_H__

/* ===== Dependencies ===== */
#include <stdint.h>
#include "esp_wifi.h"

/* ===== Macros of public constants ===== */
#define WIFI_FIRST_CONFIG		1
#define MAX_WIFI_CONNECT_RETRY 	10
#define MAX_WIFI_SSID_SIZE 		32
#define MAX_WIFI_PASSWORD_SIZE 	64

/* ===== Public structs and enums ===== */
/*------------------------------------------------------------------
|  Struct: wifi_credential_t
| ------------------------------------------------------------------
|  Description: stores the SSID and password associated to a WiFi
|				network.
|
|  Members:
|		ssid 		- SSID of the WiFi network
|		password 	- password of the wifi network
*-------------------------------------------------------------------*/
typedef struct {
	uint8_t ssid[32];
	uint8_t password[64];
} wifi_credential_t;


/* ===== Prototypes of public functions ===== */
/*------------------------------------------------------------------
|  Function: initialize_wifi
| ------------------------------------------------------------------
|  Description: it configures WiFi in STATION mode, initializes
|				the WiFi driver and starts WiFi.
|
|  Parameters:
|		- first_time: flag to be used the first time it is called.
|		- wifi_mode: WiFi mode to initialize.
|
|  Returns:  void
*-------------------------------------------------------------------*/
void initialize_wifi(uint8_t first_time, wifi_mode_t wifi_mode, wifi_credential_t* wifi_credential);


/*------------------------------------------------------------------
|  Function: stop_wifi
| ------------------------------------------------------------------
|  Description: it disconnects the WiFi and stops the WiFi driver.
|
|  Parameters:
|		-
|
|  Returns:  void
*-------------------------------------------------------------------*/
void stop_wifi();

/*------------------------------------------------------------------
|  Function: get_current_wifi_credentials
| ------------------------------------------------------------------
|  Description: returns the current WiFi credentials.
|
|  Parameters:
|		-
|
|  Returns: wifi_credential_t* - pointer to the current WiFi
|								 credentials.
*-------------------------------------------------------------------*/
wifi_credential_t* get_current_wifi_credentials();

/*------------------------------------------------------------------
|  Function: set_current_wifi_credentials
| ------------------------------------------------------------------
|  Description: sets the current WiFi credentials.
|
|  Parameters:
|		- char* ssid: new WiFi SSID.
|		- char* password: new WiFi password.
|
|  Returns: void
*-------------------------------------------------------------------*/
void set_current_wifi_credentials(char* ssid, char* password);


/* ===== Avoid multiple inclusion ===== */
#endif // __WIFI_H__
