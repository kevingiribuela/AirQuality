#include <stdio.h>
#include "co2.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void scd30_read_variables(float* temperature, float* humidity, float* co2){
    ;
}

void scd30_continuous_measurement(void){
    i2c_cmd_handle_t cmd;
    cmd = i2c_cmd_link_create();                                        // Create and initialize an I2C commands list

    i2c_master_start(cmd);                                              // Start I2C master mode
    i2c_master_write_byte(cmd, (SCD30_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, SCD30_CONTINUOUS_MEASUREMENT_MSB, true); // MSB Data ready command
    i2c_master_write_byte(cmd, SCD30_CONTINUOUS_MEASUREMENT_LSB, true); // LSB Data ready command
    i2c_master_write_byte(cmd, 0x00, true);     // No pressure compensation MSB
    i2c_master_write_byte(cmd, 0x00, true);     // No pressure compensation LSB
    i2c_master_write_byte(cmd, 0x81, true);     // CRC
    i2c_master_stop(cmd);                                               // Stop signal
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);       // Send queue data

    i2c_cmd_link_delete(cmd);                                           // Free resources
}

int scd30_data_ready_status(){
    i2c_cmd_handle_t cmd;
    uint8_t buff[3];

    cmd = i2c_cmd_link_create();                                    // Create and initialize an I2C commands list

    // Writing
    i2c_master_start(cmd);                                          // Start signal
    i2c_master_write_byte(cmd, (SCD30_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);   // Send queue data
    vTaskDelay(30/portTICK_PERIOD_MS);
    i2c_master_write_byte(cmd, SCD30_DATA_READY_MSB, true);         // MSB Data ready command
    i2c_master_write_byte(cmd, SCD30_DATA_READY_LSB, true);         // LSB Data ready command
    i2c_master_stop(cmd);                                           // Stop signal
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);   // Send queue data

    vTaskDelay(30/portTICK_PERIOD_MS);

    // Reading
    i2c_master_start(cmd);                                          // Start signal
    i2c_master_write_byte(cmd, (SCD30_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);   // Send queue data
    vTaskDelay(30/portTICK_PERIOD_MS);
    i2c_master_read_byte(cmd, &buff[2], I2C_MASTER_ACK);            // Storage data
    i2c_master_read_byte(cmd, &buff[1], I2C_MASTER_ACK);            // Storage data
    i2c_master_read_byte(cmd, &buff[0], I2C_MASTER_ACK);            // Storage data
    i2c_master_stop(cmd);                                           // Stop signal
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);   // Send queue data
    
    i2c_cmd_link_delete(cmd);                                       // Free resources

    printf("buff[2]=%x \n",buff[2]);
    printf("buff[1]=%x \n",buff[1]);
    printf("buff[0]=%x \n",buff[0]);
    if(buff[1]==1){
        return 1;
    }
    else{
        return 0;
    }
}