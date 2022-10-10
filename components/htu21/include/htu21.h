#include "esp_log.h"

// Temp & Hum sensor parametters
#define HTU21_SENSOR_ADDR            0x40                        /*!< Slave address of the GY21 module sensor */
#define HTU21_MEASURING_TIME_MS      50                          /*!< Max measuring time  in mS (datasheet)*/
#define HTU21_READ_TEMP               0xF3                        /*!< Read temperature register */
#define HTU21_READ_HUM                0xF5                        /*!< Read humidity register */


typedef int esp_err_t;

/* Read temperature or humidity */ 
uint16_t htu21_read(uint8_t command);

/* Get temperature */
float get_temp();

/* Get temperature */
float get_hum();

/* Verify CRC */ 
bool is_crc_valid(uint16_t value, uint8_t crc);
