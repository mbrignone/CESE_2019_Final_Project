/* ===== [i2c_master.h] =====
 * Copyright Matias Brignone <mnbrignone@gmail.com>
 * All rights reserved.
 *
 * Version: 0.1.0
 * Creation Date: 2019
 */

/* ===== Avoid multiple inclusion ===== */
#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

/* ===== Dependencies ===== */
#include "driver/i2c.h"

/* ===== Macros of public constants ===== */
#define I2C_MASTER_NUM      I2C_NUM_1   // I2C assigned to the master

/* ===== Public structs and enums ===== */

/* ===== Prototypes of public functions ===== */

/*------------------------------------------------------------------
|  Function: initialize_i2c_master
| ------------------------------------------------------------------
|  Description: it configures the I2C master port using default
|				GPIOs.
|
|  Parameters:
|		-
|
|  Returns:  esp_err_t
*-------------------------------------------------------------------*/
esp_err_t initialize_i2c_master();


/*------------------------------------------------------------------
|  Function: i2c_master_task
| ------------------------------------------------------------------
|  Description: FreeRTOS task. It periodically reads the I2C slave,
|				if there is a new command, it passes it to the
|				WiFi task.
|
|  Parameters:
|		- pvParameter: void pointer used as task parameter during
|					   task creation.
|
|  Returns:  void
*-------------------------------------------------------------------*/
void i2c_master_task(void *pvParameter);


/*------------------------------------------------------------------
|  Function: i2c_master_read_slave
| ------------------------------------------------------------------
|  Description:
|
|  Parameters:
|		-
|
|  Returns:  void
*-------------------------------------------------------------------*/
esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint16_t slave_address, uint8_t *data_rd, size_t size);


/*------------------------------------------------------------------
|  Function: i2c_master_write_slave
| ------------------------------------------------------------------
|  Description:
|
|  Parameters:
|		-
|
|  Returns:  void
*-------------------------------------------------------------------*/
esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint16_t slave_address, uint8_t *data_wr, size_t size);

/* ===== Avoid multiple inclusion ===== */
#endif // __I2C_MASTER_H__
