idf_component_register(SRCS "MQTT_lib.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash mqtt gpio)
