#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <sys/param.h>
#include <esp_http_server.h>
#include "WiFi.h"
#include "WebServer.h"
#include "stdbool.h"
#include "uart_scd.h"

#include "scd30.h"
#include "bh1750.h"                 // Lum Sensor
#include "gpio.h"                   // GPIO driver
#include "i2c.h"
#include "MQTT_lib.h"
#include "ctype.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"

/* Definición de tópicos raiz*/
#define LUX_TOPIC "AirQuality/Luminosidad_"
#define HUM_TOPIC "AirQuality/Humedad_"
#define TEMP_TOPIC "AirQuality/Temperatura_"
#define CO2_TOPIC "AirQuality/CO2_"

/* Bits de señalización para los eventos WiFi en RTOS */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/* Parametros que definen el numero de reintentos y tiempo de espera */
#define MAX_RETRY     20        // Number of retry to connect to WiFi
#define TIME_OUT_WIFI 120       // Time before connect with default configuration

/* A continuación, variables globales utilizadas por diferentes bibliotecas para comunicarse 
con las funciones cuando suceden diferentes eventos */
bool wifi_ok = false, start_connection = false;
bool mqtt_ok = false;
bool parametters = false;
bool loop = false;
bool acces_point=true;
int retry_conn = 0, i=0;
size_t required_size;
char TOPIC_LUX[35]= "", TOPIC_CO2[35] = "", TOPIC_TEMP[35] = "", TOPIC_HUM[35] = "";
float CO2, temperature, relative_humidity;

nvs_handle_t my_handle;
EventGroupHandle_t s_wifi_event_group;
esp_netif_t *sta_object, *ap_object;
esp_mqtt_client_handle_t client = NULL;

/* Funcion sencilla que convierte cualquier cadena a mayusculas */
void str_capitol(char* cadena)
{
    for(int i=0; cadena[i]!='\0';i++){
        cadena[i]=toupper((const char)cadena[i]);
    }
    return;
}

/* Funcion que lee memoria no-volatil para obtener el nombre del aula donde esta colocado
* el AirQuality asi prepara los topicos acorde al aula. Luego, imprime por puerta serie el resultado
* de como quedaron formados los topicos */
void prepare_topics(void){
    nvs_open("wifi",NVS_READWRITE, &my_handle);         // Open the nvs in read/write mode

    nvs_get_str(my_handle, "AULA", NULL, &required_size);  // Get the required size, and value of the PSWD from NVS
    char *aula = malloc(required_size);
    nvs_get_str(my_handle, "AULA", aula, &required_size);

    str_capitol(aula);

    strcpy(TOPIC_LUX,LUX_TOPIC);
    strcat(TOPIC_LUX,aula);

    strcpy(TOPIC_TEMP,TEMP_TOPIC);
    strcat(TOPIC_TEMP,aula);

    strcpy(TOPIC_HUM,HUM_TOPIC);
    strcat(TOPIC_HUM,aula);

    strcpy(TOPIC_CO2,CO2_TOPIC);
    strcat(TOPIC_CO2,aula);

    printf("TOPICOS:\n");
    printf("%s\n",TOPIC_HUM);
    printf("%s\n",TOPIC_TEMP);
    printf("%s\n",TOPIC_LUX);
    printf("%s\n\n",TOPIC_CO2);

    free(aula);
    nvs_close(my_handle);
}

/* void wifi_event_handler
* Se trata del manejador de enventos WiFi, sus parámetros son el evento base, id del evento, y el dato que 
* aporta dicho evento. 
* WIFI_EVENT: Si se trata de eventos WiFi analiza si se ha iniciado en modo STA, si conectó como
*             STA o se desconectó del STA, o si simplemente la conexión falló. 
*               1 - WIFI_EVENT_STA_START: Inicia el protocolo de conexión.
*               2 - WIFI_EVENT_STA_CONNECTED: Indica que se pudo conectar al AP.
*               3 - WIFI_EVENT_STA_DISCONNECTED: Indica que se desconectó del AP, entonces inicia el proceso
*                                               de reconexion a la red hasta un maximo de MAX_RETRY intentos.
*                                               Si no se puede conectar, vuelve a modo AP reseteando el ESP.
*               4 - WIFI_EVENT_AP_START: Se inició el AP.
* IP_EVENT: Al ocurrir un evento con el IP, el único que puede suceder es que se obtuvo un IP para el 
*           dispositivo, de esta forma, se indica con un led verde que estamos conectados a WiFi. */
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if(event_base == WIFI_EVENT){
        if (event_id == WIFI_EVENT_STA_START){
            vTaskDelay(100/portTICK_RATE_MS);
            printf("CONNECTING WIFI...\n");
            esp_err_t wifi = esp_wifi_connect();    // Connecting
            if(wifi!=ESP_OK){
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                printf("WIFI NOT CONNECTD...\n");
            }
            else{
                printf("WIFI CONNECTED...\n\n");
            }
            
        }
        else if(event_id == WIFI_EVENT_STA_CONNECTED){
            printf("Conexión a AP exitosa!\n");
        }
        else if(event_id == WIFI_EVENT_STA_DISCONNECTED){
            wifi_ok=false;
            if(retry_conn<MAX_RETRY){
                if(loop==true){
                    esp_wifi_connect(); // Trying to reconnect
                    retry_conn++;
                    printf("Intento de reconexion Nro: %d de %d\n", retry_conn, MAX_RETRY);
                    for(i=0; i<5;i++){
                        vTaskDelay(1000/portTICK_PERIOD_MS);
                    }
                }
            }
            else{
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT); // Flag to reset ESP
                retry_conn=0;
            }
        }
        else if(event_id == WIFI_EVENT_AP_START){
            blue_led(ON);       // Acces Point started
            acces_point=true;
        }
        else if(event_id == WIFI_EVENT_AP_STOP){
            blue_led(OFF);      // Acces Point stoped
            acces_point=false;
        }
    }
    else if(event_base == IP_EVENT){
        if(event_id == IP_EVENT_STA_GOT_IP){
            green_led(ON);
            wifi_ok=true;
            retry_conn=0;
            printf("IP obtained!\n\n");
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }      
    }
}

void vWiFi_disconnect(void *pvParameters){
    while (1){
        if(!(wifi_ok||acces_point)){
            red_led(ON);
            vTaskDelay(300/portTICK_PERIOD_MS);
            red_led(OFF);
            vTaskDelay(300/portTICK_PERIOD_MS);
        }
        else{
            vTaskDelay(5/portTICK_PERIOD_MS);
        }
    }
}


void app_main(void)
{
    int time_out=TIME_OUT_WIFI;
    uint16_t lux;
    char buff[90];  
    bool data_R;

    static httpd_handle_t server = NULL;
    s_wifi_event_group = xEventGroupCreate(); // Create event group for wifi events

    configure_led();    // Configure GPIO ports for leds

    ESP_ERROR_CHECK(i2c_master_init());
    printf("\nI2C initialized successfully\n");

    uart_scd_init();
    printf("\nUART initialized successfully\n");

   setMeasurementInterval(0x0002);      // Setting measurement interval for CO2 sensor
   startContinuousMeasurement(0x0000);  // Trigger continuous measurement
   
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    printf("\nNVS initialized successfully\n");

    ESP_ERROR_CHECK(esp_netif_init());
    printf("\nTCP/IP Protocol initialized successfully\n");

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    printf("\nEvent loop created successfuly\n");

    /* Setting the Wi-Fi and IP handler */
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);

    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);
    
    xTaskCreate(vWiFi_disconnect, "Blink led", 1000, NULL, 1, NULL);

    while(true){
        /* Start ESP32 in Acces Point mode */
        ap_object = wifi_init_softap();
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
        
        /* Start the WebServer and wait for configurations, then, stop it and detach handlers*/
        server = start_webserver();
        while(parametters != true){
            vTaskDelay(1000/portTICK_PERIOD_MS);
            time_out--;
            if(time_out==0){
                time_out=TIME_OUT_WIFI;
                parametters=true;

                nvs_open("wifi",NVS_READWRITE, &my_handle);         // Open the nvs in read/write mode
                
                nvs_get_str(my_handle, "SSID", NULL, &required_size);  // Get the required size, and value of the SSID from NVS
                char *wifi_ssid = malloc(required_size);
                nvs_get_str(my_handle, "SSID", wifi_ssid, &required_size);
                
                nvs_get_str(my_handle, "PSWD", NULL, &required_size);  // Get the required size, and value of the PSWD from NVS
                char *wifi_pswd = malloc(required_size);
                nvs_get_str(my_handle, "PSWD", wifi_pswd, &required_size);

                printf("Iniciando conexion default...\n");
                printf("SSID: %s\n", wifi_ssid);
                printf("PSWD: %s\n\n", wifi_pswd);

                nvs_close(my_handle);
                free(wifi_pswd);
                free(wifi_ssid);
            }
        }
        parametters=false;
        time_out = TIME_OUT_WIFI;
        stop_webserver(server);
        
        ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler));
        ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler));
        
        /* Start ESP32 in Station mode*/ 
        sta_object = wifi_init_sta();
        
        /* Start MQTT client */ 
        if(wifi_ok){    // If WiFi connection is ready, then start mqqt client and prepare topics
            client=mqtt_app_start();
            prepare_topics();
            loop = true;
        }
        else{           // Else, go to AP again
            loop=false;
        }

        while(loop){
            if((wifi_ok==true) && (mqtt_ok==true)){
                if((data_R=dataReady())){
                    datardos();         // Get temp, hum and co2 values
                    lux = get_lux();    // Get lux value

                    printf("Temperatura: %f\r\nHumedad: %f\r\nLumenes: %i\r\nCO2: %f\n\n", temperature, relative_humidity, lux,CO2);

                    /* Sent over MQTT to beebotte the data collected */
                    sprintf(buff,"{\"data\": %.2f, \"write\": true, \"ispublic\": true}",temperature);
                    esp_mqtt_client_publish(client, TOPIC_TEMP, buff, 0, 1, 0);

                    sprintf(buff,"{\"data\": %.2f, \"write\": true, \"ispublic\": true}",relative_humidity);
                    esp_mqtt_client_publish(client, TOPIC_HUM, buff, 0, 1, 0);

                    sprintf(buff,"{\"data\": %i, \"write\": true, \"ispublic\": true}",lux);
                    esp_mqtt_client_publish(client, TOPIC_LUX, buff, 0, 1, 0);

                    sprintf(buff,"{\"data\": %.2f, \"write\": true, \"ispublic\": true}",CO2);
                    esp_mqtt_client_publish(client, TOPIC_CO2, buff, 0, 1, 0);

                    /* If co2 levels got out admitted range, trigger warnings */
                    if(CO2<600){                    // Chill
                        green_led(ON);
                    }
                    else if((CO2>600)&&(CO2<1000)){ // Warning...
                        yellow_led(ON);
                    }
                    else{                           // Dangerous!
                        red_led(ON);
                    }
                }
            }
            /* If something is wrong with WiFi, externals handlers set wifi_ok to FALSE, then, disable mqtt protocol
            * destroy their handlers and then wait undefinitely until some new WiFi event occurs.
            * If WiFi connection is OK, then continue normaly. 
            * Else, restart the ESP and put in AP mode again. */
            else{
                if(wifi_ok==false){
                    mqtt_ok=false;
                    esp_mqtt_client_stop(client);
                    esp_mqtt_client_destroy(client);
                    
                    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                        pdTRUE,
                                        pdFALSE,
                                        portMAX_DELAY);
                    if(bits & WIFI_CONNECTED_BIT){
                        wifi_ok = true;
                        client=mqtt_app_start();               
                    }
                    else if(bits & WIFI_FAIL_BIT){
                        loop=false;
                    }
                }
            }
        }  
        esp_wifi_stop();
        esp_netif_destroy_default_wifi(sta_object);
        esp_netif_destroy_default_wifi(ap_object);
    }
}
