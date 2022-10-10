#include "esp_log.h"

#define SCD30_SENSOR_ADDR          0x61    /*!< Slave address of the BH1750 module sensor */
#define SCD30_CONTINUOUS_MEASUREMENT_MSB 0x00
#define SCD30_CONTINUOUS_MEASUREMENT_LSB 0x10
#define SCD30_STOP_MEASUREMENT 0x0104
#define SCD30_DATA_READY_MSB 0x02
#define SCD30_DATA_READY_LSB 0x02
#define SCD30_READ_DATA 0x0300

void scd30_read_variables(float* temperature, float* humidity, float* co2);

void scd30_continuous_measurement(void);

int scd30_data_ready_status(void);
