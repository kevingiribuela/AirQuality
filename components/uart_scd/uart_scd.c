#include <stdio.h>
#include "uart_scd.h"
#include "driver/uart.h"

#define UART_RX         16
#define UART_TX         17 

//Configure parameters of the UART driver, communication pins and install the driver
void uart_scd_init(void)
{
    const int uart_buffer_size = 4*1024;
    uart_config_t uart_config = {
        .baud_rate = 19200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
    };
    vTaskDelay(1000/portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, UART_TX, UART_RX, -1, -1));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uart_buffer_size, 0, 0, NULL, 0));
}
