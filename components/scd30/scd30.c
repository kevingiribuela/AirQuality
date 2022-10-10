#include <stdio.h>
#include "scd30.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include <string.h>

extern unsigned char CO2_raw[4], temperature_raw[4], relative_humidity_raw[4];
extern float CO2, temperature, relative_humidity;

unsigned char* calculateCRC(unsigned char *buf, uint16_t len) {
   static unsigned char crc[2];

   uint16_t crc_int = 0xFFFF;
   for (uint16_t pos = 0; pos < len; pos++){
      crc_int ^= (uint16_t)buf[pos];         // XOR byte into least sig. byte of crc

      for (uint16_t i = 8; i != 0; i--) {    // Loop over each bit
         if ((crc_int & 0x0001) != 0) {      // If the LSB is set
            crc_int >>= 1;                   // Shift right and XOR 0xA001
            crc_int ^= 0xA001;
         }
         else                                // Else LSB is not set
            crc_int >>= 1;                   // Just shift right
      }
   }
   //printf("CRC: %X\n",crc_int);
   crc[0] = crc_int & 0x00FF;
   crc[1] = (crc_int & 0xFF00) >> 8;

   return crc;
}

_Bool checkCRC(unsigned char *buf, uint16_t len) {
   uint8_t target_length = len - 2;
   unsigned char target_buf[target_length];
   unsigned char *calculated_crc;

   for ( uint8_t i = 0; i < target_length; i++ ) {
      target_buf[i] = buf[i];
   }

   calculated_crc = calculateCRC(target_buf, target_length);

   if (calculated_crc[1] == buf[len-1] && calculated_crc[0] == buf[len-2]) {
      return true;
   } else {
      return false;
   }
}

void scd30Write(uint8_t func, uint16_t cmd, uint16_t data) {

   unsigned char buffer[6];
   unsigned char buffer_aux[8];
   unsigned char *crc;
   buffer[0] = SCD30_MODBUSADDR_DEFAULT;
   buffer[1] = func;
   buffer[2] = (cmd & 0xFF00) >> 8;
   buffer[3] = cmd & 0x00FF;
   buffer[4] = (data & 0xFF00) >> 8;
   buffer[5] = data & 0x00FF;

   crc = calculateCRC(buffer, 6);
   for(int i=0; i<6; i++){
      buffer_aux[i]=buffer[i];
   }
   buffer_aux[6]=crc[0];
   buffer_aux[7]=crc[1];
   //printf("Dato enviado:\t %X %X %X %X %X %X %X %X\n",buffer_aux[0], buffer_aux[1], buffer_aux[2], buffer_aux[3], buffer_aux[4], buffer_aux[5], buffer_aux[6], buffer_aux[7]);
   uart_write_bytes(UART_NUM_2, buffer_aux, sizeof(buffer_aux));
}

_Bool scd30Read(unsigned char *buf, uint8_t bytes) {
   uart_read_bytes(UART_NUM_2, buf, bytes, 5000/ portTICK_RATE_MS);
   /*if(bytes !=8){
        printf("Datos recibidos: %X %X %X %X %X %X %X\n",buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
    }
    else{
        printf("Datos recibidos: %X %X %X %X %X %X %X %X\n",buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    }*/
   
   if (checkCRC(buf, bytes)) {
        //printf("CheckSum CORRECTO\n");
        return true;
   }
   else{
        //printf("ChekSum INCORRECTO\n");
        return false;
   }
}

_Bool dataReady(void) {
    uint8_t len = 7;
    unsigned char response[len];
    
    scd30Write(0x03, SCD30_CMD_GET_DATA_READY, 0x0001);

    vTaskDelay(3000/portTICK_PERIOD_MS);

    if (scd30Read(response, len)){
        if (response[4] == 0x01){
        return true;
    }
    else{
        return false;
    }
    }
   else{
        printf("No se puede leer el buffer RX.\n");
        return false;
   }
}

_Bool datardos(void) {
   uint8_t len = 17;
   unsigned char response[len];
   scd30Write(0x03, SCD30_CMD_READ_MEASUREMENT, 0x0006);
   vTaskDelay(3000/portTICK_PERIOD_MS);
  
   if (scd30Read(response, len)) {
      // CRCs are good, unpack floats
      uint32_t co2 = 0, temp = 0, hum = 0;

      co2 |= (unsigned int)response[3];
      co2 <<= 8;
      co2 |= (unsigned int)response[4];
      co2 <<= 8;
      co2 |= (unsigned int)response[5];
      co2 <<= 8;
      co2 |= (unsigned int)response[6];

      temp |= (unsigned int)response[7];
      temp <<= 8;
      temp |= (unsigned int)response[8];
      temp <<= 8;
      temp |= (unsigned int)response[9];
      temp <<= 8;
      temp |= (unsigned int)response[10];

      hum |= (unsigned int)response[11];
      hum <<= 8;
      hum |= (unsigned int)response[12];
      hum <<= 8;
      hum |= (unsigned int)response[13];
      hum <<= 8;
      hum |= (unsigned int)response[14];

      memcpy(&CO2, &co2, sizeof(co2));
      memcpy(&temperature, &temp, sizeof(temp));
      memcpy(&relative_humidity, &hum, sizeof(hum));

      return true;
   }
   else{
      return false;
   }
}

_Bool setMeasurementInterval(uint16_t interval) {
   uint8_t len = 8;
   unsigned char response[len];

   scd30Write(0x06, SCD30_CMD_SET_MEASUREMENT_INTERVAL, interval);

   vTaskDelay(10/portTICK_PERIOD_MS);

   if ( scd30Read(response, len) ) {
      if ( (response[4]==(interval & 0xFF00)>>8) && (response[5]==(interval & 0x00FF)) ) {
        return true;
        }
        else{
            return false;
        }
    }
   else{
      return false;
   }
}

_Bool startContinuousMeasurement(uint16_t pressure) {
    uint8_t len  = 8;
    unsigned char response[len];

    scd30Write(0x06, SCD30_CMD_CONTINUOUS_MEASUREMENT, pressure);

    vTaskDelay(10/portTICK_PERIOD_MS);

    if(scd30Read(response, len)){
        if((response[4]==(pressure & 0xFF00)>>8) && (response[5]==(pressure & 0x00FF))){
            return true;
        } 
        else{
            return false;
        }
    }
   else{
        return false;
    }
}

_Bool stopContinuousMeasurement() {
    uint8_t len = 8;
    unsigned char response[len];

    scd30Write(0x06, SCD30_CMD_STOP_MEASUREMENTS, 0x001);

    vTaskDelay(5/portTICK_PERIOD_MS);

    if( scd30Read(response, len) ){
        if((response[4]==0x00) && (response[5]==0x01)){
            return true;
        }
        else{
            return false;
        }
    }
    else{
        return false;
    }
}
