#include "driver/gpio.h"            // GPIO driver

// LED
#define BLUE_LED 25
#define RED_LED 32
#define GREEN_LED 33
#define ON 1
#define OFF 0

void configure_led(void);
void blue_led(int state);
void red_led(int state);
void green_led(int state);
void yellow_led(int state);
void leds_off(void);
