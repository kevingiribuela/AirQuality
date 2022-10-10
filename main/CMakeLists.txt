idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES co2 MQTT_lib mqtt json i2c gpio htu21 bh1750 WiFi nvs_flash esp_http_server WebServer scd30 uart_scd)
