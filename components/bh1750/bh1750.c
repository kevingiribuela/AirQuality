#include <stdio.h>
#include "bh1750.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

uint16_t get_lux(void){
    esp_err_t ret;
    i2c_cmd_handle_t cmd;

    // Send read command
    cmd = i2c_cmd_link_create();                       // Create and initialize an I2C commands list
    i2c_master_start(cmd);                                              // Start I2C master mode
    i2c_master_write_byte(cmd, (BH1750_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, BH1750_READ_LUX, true);                          // Put bytes commands on queue
    i2c_master_stop(cmd);                                               // Stop I2c master mode
    ret=i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);  // Send queue data
    i2c_cmd_link_delete(cmd);                                           // Free resources
    if(ret!=ESP_OK) return 0;

    // Wait for sensor
    vTaskDelay(BH1750_MEASURING_TIME_MS/portTICK_RATE_MS);

    // Receive data from sensor
    uint8_t msb, lsb;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BH1750_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &msb, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &lsb, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    ret=i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if(ret!=ESP_OK) return 0;

    // Return data
    uint16_t data = ((uint16_t) msb<<8) | ((uint16_t)lsb);
    return data;
}