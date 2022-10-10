#include "esp_log.h"

#define BH1750_SENSOR_ADDR          0x23    /*!< Slave address of the BH1750 module sensor */
#define BH1750_READ_LUX             0x20    /*!< Start measurement at 1lx of resolution */
#define BH1750_MEASURING_TIME_MS    180     /*!< Max time measurement */

uint16_t get_lux(void);