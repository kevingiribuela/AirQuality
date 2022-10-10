#include <stdio.h>
#include "htu21.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


/* Read value */ 
uint16_t htu21_read(uint8_t command)
{
    esp_err_t ret;
    i2c_cmd_handle_t cmd;
    // Send command
    cmd = i2c_cmd_link_create();                       // Create and initialize an I2C commands list
    i2c_master_start(cmd);                                              // Start I2C master mode
    i2c_master_write_byte(cmd, (HTU21_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, command, true);                          // Put bytes commands on queue
    i2c_master_stop(cmd);                                               // Stop I2c master mode
    ret=i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);  // Send queue data
    i2c_cmd_link_delete(cmd);                                           // Free resources
    if(ret!=ESP_OK) return 0;

    // Wait for sensor
    vTaskDelay(50/portTICK_RATE_MS);

    // Receive data from sensor
    uint8_t msb, lsb, crc, error=0;
    uint16_t status;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HTU21_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &msb, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &lsb, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &crc, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    ret=i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if(ret!=ESP_OK) return 0;

    status=((uint16_t)msb<<8) |((uint16_t)lsb);
    switch (status)
    {
    case 0xFFF:
        printf("Short circuit.\r\n");
        error=1;
        break;

    case 0x0000:
        printf("Open circuit.\r\n");
        error=1;
        break;

    default:
        error=ESP_OK;
        break;
    }
    if(error!=ESP_OK) return 0;

    // Verifying CheckSum
    uint16_t data = ((uint16_t) msb<<8) | ((uint16_t)lsb);
    if(!is_crc_valid(data,crc))
    {   
        printf("CRC invalid.\r\n");
        return 0;
    }

    // Return data
    return data&0xFFFC;       // data & 1111 1111 1111 1100 --> bit 0 and bit 1, status bits.
}

/* Get temperature */
float get_temp()
{
    float temp=htu21_read(HTU21_READ_TEMP);
    if(temp==0) return -999;
    else return (temp * 175.72 / 65536.0) - 46.85;
}

/* Get temperature */
float get_hum()
{
    float hum=htu21_read(HTU21_READ_HUM);
    if(hum==0) return -999;
    else return (hum * 125.0 / 65536.0) - 6.0;
}

/* Verify CRC routine */
bool is_crc_valid(uint16_t value, uint8_t crc)
{	
	uint32_t row = (uint32_t)value << 8;
	row |= crc;
	
	// Pol = x^8 + x^5 + x^4 + 1 padded with zeroes corresponding to the bit length of the CRC
	uint32_t divisor = (uint32_t)0x988000;
	
	for (int i = 0 ; i < 16 ; i++) {
		
		// if the input bit above the leftmost divisor bit is 1, 
		// the divisor is XORed into the input
		if (row & (uint32_t)1 << (23 - i)) row ^= divisor;
		
		// the divisor is then shifted one bit to the right
		divisor >>= 1;
	}
	
	// the remainder should equal zero if there are no detectable errors
	return (row == 0);
}