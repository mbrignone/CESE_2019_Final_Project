/* ===== [slave_sim_task.c] =====
 * Copyright Matias Brignone <mnbrignone@gmail.com>
 * All rights reserved.
 *
 * Version: 0.1.0
 * Creation Date: 2019
 */


/* ===== Dependencies ===== */
#include "slave_sim_task.h"
#include "serial_protocol_common.h"
#include "command_processor.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

/* ===== Macros of private constants ===== */
#define I2C_SLAVE_NUM           I2C_NUM_0
#define I2C_SLAVE_TX_BUF_LEN    1024
#define I2C_SLAVE_RX_BUF_LEN    1024
#define SLAVE_PROCESS_A_TIME    10
#define SLAVE_PROCESS_B_TIME    5

/* ===== Declaration of private or external variables ===== */
static const char *TAG = "I2C_SLAVE_SIM_TASK";
static slave_machine_state_t slave_state;

/* ===== Prototypes of private functions ===== */
esp_err_t initialize_i2c_slave(uint16_t slave_addr);
slave_machine_state_t update_slave_sim_fsm(slave_machine_state_t current_state, uint8_t current_cmd);

/* ===== Implementations of public functions ===== */
void slave_sim_task(void *pvParameter)
{
    uint8_t command_frame[COMMAND_FRAME_LENGTH];
    uint8_t command_data;
    size_t sent_size;
    int32_t slave_read_buffer_size;
    esp_err_t error_state;

    // set initial state
    slave_state = SLAVE_IDLE;

    // initialize I2C slave
    error_state = initialize_i2c_slave(I2C_ESP_SLAVE_ADDR);
    if (error_state != ESP_OK)
    {
        ESP_LOGE(TAG, "Error initializing I2C slave.");
        // do something about this
    }

    while (1)
    {
        // read data from slave buffer
        slave_read_buffer_size = i2c_slave_read_buffer(I2C_SLAVE_NUM, command_frame, COMMAND_FRAME_LENGTH, 250 / portTICK_RATE_MS);
        if (slave_read_buffer_size > 0 && check_frame_format(command_frame))
        {
            // ESP_LOGI(TAG, "I2C Slave Sim Task read from slave buffer: %d\n", command_frame[1]);
            command_data = command_frame[1];

            // send back to the master a message indicating that the command was correctly received
            command_frame[1] = CMD_SLAVE_OK;
            sent_size = i2c_slave_write_buffer(I2C_SLAVE_NUM, command_frame, COMMAND_FRAME_LENGTH, 500 / portTICK_RATE_MS);
            if (sent_size == 0)
            {
                printf("I2C slave buffer is full, slave UNABLE to write master\n");
            }
        }
        else
        {
            command_data = 255;
        }

        // ESP_LOGI(TAG, "Current SLAVE STATE: %d.\n", (uint8_t)slave_state);

        slave_state = update_slave_sim_fsm(slave_state, command_data);

        vTaskDelay(250 / portTICK_RATE_MS);
    }
}


/* ===== Implementations of private functions ===== */
esp_err_t initialize_i2c_slave(uint16_t slave_addr)
{
    int i2c_slave_port = I2C_SLAVE_NUM;
    i2c_config_t i2c_slave_config = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = I2C_SLAVE_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SLAVE_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave = {
            .addr_10bit_en = 0,
            .slave_addr = slave_addr,
        },
    };

    i2c_param_config(i2c_slave_port, &i2c_slave_config);
    return i2c_driver_install(i2c_slave_port, i2c_slave_config.mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
}


slave_machine_state_t update_slave_sim_fsm(slave_machine_state_t current_state, uint8_t current_cmd)
{
    static uint8_t state_time_counter           = 0;
    static slave_machine_state_t paused_state   = SLAVE_IDLE;
    uint8_t command_frame[COMMAND_FRAME_LENGTH] = {COMMAND_FRAME_START, 0, COMMAND_FRAME_END};
    size_t sent_size;

    switch (current_state)
    {
    case SLAVE_IDLE:
        if(current_cmd == CMD_SLAVE_START_A)
        {
            ESP_LOGI(TAG, "Slave IDLE received command A, switching to SLAVE_PROCESS_A.\n");
            current_state = SLAVE_PROCESS_A;
            state_time_counter = 0;
        }
        else if(current_cmd == CMD_SLAVE_START_B)
        {
            ESP_LOGI(TAG, "Slave IDLE received command B, switching to SLAVE_PROCESS_B.\n");
            current_state = SLAVE_PROCESS_B;
            state_time_counter = 0;
        }

        break;
    case SLAVE_PROCESS_A:
        if(state_time_counter == 0)
        {
            ESP_LOGI(TAG, "Entered into SLAVE_PROCESS_A.\n");
        }
        state_time_counter++;
        if(state_time_counter == SLAVE_PROCESS_A_TIME)
        {
            ESP_LOGI(TAG, "Slave PROCESS_A finished, switching to SLAVE_DONE.\n");
            current_state = SLAVE_DONE;

            // notify master that the process finished
            command_frame[1] = CMD_SLAVE_START_A;
            sent_size = i2c_slave_write_buffer(I2C_SLAVE_NUM, command_frame, COMMAND_FRAME_LENGTH, 500 / portTICK_RATE_MS);
            if (sent_size == 0)
            {
                printf("I2C slave buffer is full, slave UNABLE to write master\n");
            }
        }
        else if(current_cmd == CMD_SLAVE_PAUSE)
        {
            ESP_LOGI(TAG, "Slave PROCESS_A received command P, switching to SLAVE_PAUSE.\n");
            paused_state = current_state;
            current_state = SLAVE_PAUSE;
        }
        else if(current_cmd == CMD_SLAVE_RESET)
        {
            ESP_LOGI(TAG, "Slave PROCESS_A received command R, switching to SLAVE_IDLE.\n");
            current_state = SLAVE_IDLE;
        }

        break;
    case SLAVE_PROCESS_B:
        if(state_time_counter == 0)
        {
            ESP_LOGI(TAG, "Entered into SLAVE_PROCESS_B.\n");
        }
        state_time_counter++;
        if(state_time_counter == SLAVE_PROCESS_B_TIME)
        {
            ESP_LOGI(TAG, "Slave PROCESS_B finished, switching to SLAVE_DONE.\n");
            current_state = SLAVE_DONE;

            // notify master that the process finished
            command_frame[1] = CMD_SLAVE_START_B;
            sent_size = i2c_slave_write_buffer(I2C_SLAVE_NUM, command_frame, COMMAND_FRAME_LENGTH, 500 / portTICK_RATE_MS);
            if (sent_size == 0)
            {
                printf("I2C slave buffer is full, slave UNABLE to write master\n");
            }
        }
        else if(current_cmd == CMD_SLAVE_PAUSE)
        {
            ESP_LOGI(TAG, "Slave PROCESS_B received command P, switching to SLAVE_PAUSE.\n");
            paused_state = current_state;
            current_state = SLAVE_PAUSE;
        }
        else if(current_cmd == CMD_SLAVE_RESET)
        {
            ESP_LOGI(TAG, "Slave PROCESS_B received command R, switching to SLAVE_IDLE.\n");
            current_state = SLAVE_IDLE;
        }

        break;
    case SLAVE_PAUSE:
        if(current_cmd == CMD_SLAVE_CONTINUE)
        {
            ESP_LOGI(TAG, "Slave PAUSE received command C, switching to previous paused state.\n");
            current_state = paused_state;
        }
        else if(current_cmd == CMD_SLAVE_RESET)
        {
            ESP_LOGI(TAG, "Slave PAUSE received command R, switching to SLAVE_IDLE.\n");
            current_state = SLAVE_IDLE;
        }

        break;

    case SLAVE_DONE:
        ESP_LOGI(TAG, "Slave DONE, switching to SLAVE_IDLE.\n");
        current_state = SLAVE_IDLE;

        break;

    default:
        break;
    }

    return current_state;
}
