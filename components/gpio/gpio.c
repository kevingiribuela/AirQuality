#include "gpio.h"

void configure_led(void)
{
    gpio_reset_pin(BLUE_LED);
    gpio_reset_pin(RED_LED);
    gpio_reset_pin(GREEN_LED);

    gpio_set_direction(BLUE_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(RED_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(GREEN_LED,GPIO_MODE_OUTPUT);
}

void blue_led(int state)
{
    gpio_set_level(RED_LED, 0);
    gpio_set_level(GREEN_LED, 0);
    gpio_set_level(BLUE_LED, state);
}
void red_led(int state)
{
    gpio_set_level(BLUE_LED, 0);
    gpio_set_level(GREEN_LED, 0);
    gpio_set_level(RED_LED, state);
}
void green_led(int state)
{
    gpio_set_level(BLUE_LED, 0);
    gpio_set_level(RED_LED, 0);
    gpio_set_level(GREEN_LED, state);
}
void yellow_led(int state)
{
    gpio_set_level(BLUE_LED, 0);
    gpio_set_level(RED_LED, state);
    gpio_set_level(GREEN_LED, state);
}
void leds_off(void)
{
    gpio_set_level(BLUE_LED, 0);
    gpio_set_level(RED_LED, 0);
    gpio_set_level(GREEN_LED, 0);
}