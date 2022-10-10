#define SCD30_MODBUSADDR_DEFAULT 0x61 
#define SCD30_CMD_READ_MEASUREMENT 0x0028
#define SCD30_CMD_CONTINUOUS_MEASUREMENT 0x0036
#define SCD30_CMD_STOP_MEASUREMENTS 0x0037
#define SCD30_CMD_SET_MEASUREMENT_INTERVAL 0x0025
#define SCD30_CMD_GET_DATA_READY 0x0027 
#define SCD30_CMD_AUTOMATIC_SELF_CALIBRATION 0x003A
#define SCD30_CMD_SET_FORCED_RECALIBRATION_REF 0x0039
#define SCD30_CMD_SET_TEMPERATURE_OFFSET 0x003B
#define SCD30_CMD_SET_ALTITUDE_COMPENSATION 0x0038
#define SCD30_CMD_SOFT_RESET 0x0034
#define SCD30_CMD_READ_REVISION 0x0020

unsigned char* calculateCRC(unsigned char *buf, uint16_t len);

_Bool checkCRC(unsigned char *buf, uint16_t len);

void scd30Write(uint8_t func, uint16_t cmd, uint16_t data);

_Bool scd30Read(unsigned char *buf, uint8_t bytes);

_Bool dataReady(void);

_Bool datardos(void);

_Bool setMeasurementInterval(uint16_t interval);

_Bool startContinuousMeasurement(uint16_t pressure);

_Bool stopContinuousMeasurement();