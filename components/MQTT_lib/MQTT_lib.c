
#include <stdio.h>
#include "MQTT_lib.h"
#include "nvs_flash.h"
#include "nvs.h"

extern bool mqtt_ok;
extern nvs_handle_t my_handle;

/* MQTT Event handler */
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        printf("MQTT_EVENT_CONNECTED\n");
        mqtt_ok = true;
        break;
    case MQTT_EVENT_DISCONNECTED:
        printf("MQTT_EVENT_DISCONNECTED\n");
        mqtt_ok = false;
        break;
    case MQTT_EVENT_ERROR:
        printf("MQTT_EVENT_ERROR\n");
        mqtt_ok = false;
    default:
        break;
    }
}

/* MQTT start */
esp_mqtt_client_handle_t mqtt_app_start(void)
{
    nvs_open("wifi",NVS_READWRITE, &my_handle);         // Open the nvs in read/write mode
    size_t required_size;
    
    nvs_get_str(my_handle, "BROKER", NULL, &required_size);  // Get the required size, and value of the BROKER from NVS
    char *broker = malloc(required_size);
    nvs_get_str(my_handle, "BROKER", broker, &required_size);

    nvs_get_str(my_handle, "USER", NULL, &required_size);  // Get the required size, and value of the token from NVS
    char *user = malloc(required_size);
    nvs_get_str(my_handle, "USER", user, &required_size);

    nvs_get_str(my_handle, "PSWD_MQTT", NULL, &required_size);  // Get the required size, and value of the PSWD from NVS
    char *mqtt_pswd = malloc(required_size);
    nvs_get_str(my_handle, "PSWD_MQTT", mqtt_pswd, &required_size);

    uint32_t port = 0;
    nvs_get_u32(my_handle, "PORT", &port);

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = broker,
        .username = user,
        .password = mqtt_pswd,
        .port = port
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    nvs_close(my_handle);
    free(broker);
    free(user);
    free(mqtt_pswd);

    return client;
}