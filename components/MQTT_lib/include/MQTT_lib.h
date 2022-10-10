#include "mqtt_client.h"
#include "gpio.h"


void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

esp_mqtt_client_handle_t mqtt_app_start(void);
