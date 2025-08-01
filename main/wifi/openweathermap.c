#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"
#include <cJSON.h>

#include "flipdot_gfx.h"
#include "bitmaps_weather.h"
#include "wifi/configuration_weberver.h"

static const char *TAG = "HTTP_CLIENT_OPENWEATHERMAP";

#define HTTP_RESPONSE_BUFFER_SIZE 1024

char *response_data = NULL;
size_t response_len = 0;
bool all_chunks_received = false;

esp_err_t get_openweathermap_data(const char *json_string) {
   
    cJSON *root = cJSON_Parse(json_string);
    cJSON *obj = cJSON_GetObjectItemCaseSensitive(root, "main");

    if(!obj) {
    ESP_LOGW(TAG, "Missing Data");
    cJSON_Delete(root);
    return ESP_FAIL;
    }

    float temp = cJSON_GetObjectItemCaseSensitive(obj, "temp")->valuedouble;
    int humidity = cJSON_GetObjectItemCaseSensitive(obj, "humidity")->valueint;
    
    cJSON_Delete(root);
    free(response_data);
    return ESP_OK;
}

static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_ON_DATA:
            response_data = realloc(response_data, response_len + evt->data_len);
            memcpy(response_data + response_len, evt->data, evt->data_len);
            response_len += evt->data_len;
            break;
        case HTTP_EVENT_ON_FINISH:
            all_chunks_received = true;
            ESP_LOGI("OpenWeatherAPI", "Received data: %s", response_data);
            get_openweathermap_data(response_data);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void openweather_api_http(void *pvParameters) {
    while(1) {
        open_weather_config_t *open_weather_config = get_open_weather_config();

        char open_weather_map_url[300];
        snprintf(open_weather_map_url,
                sizeof(open_weather_map_url),
                "%s%s%s%s%s%s",
                "http://api.openweathermap.org/data/2.5/weather?units=metric&q=",
                open_weather_config->city,
                ",",
                open_weather_config->country_code,
                "&appid=",
                open_weather_config->api_key);

        ESP_LOGI(TAG, "%s",open_weather_map_url);

        esp_http_client_config_t config = {
            .url = open_weather_map_url,
            .method = HTTP_METHOD_GET,
            .event_handler = _http_event_handler,
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);

        esp_err_t err = esp_http_client_perform(client);

        if(err == ESP_OK) {
            int status_code = esp_http_client_get_status_code(client);
            if(status_code == 200) {
                ESP_LOGI(TAG, "Message sent Successfully");
            }
            else {
                ESP_LOGI(TAG, "Message sent Failed");
            }
        }
        else {
            ESP_LOGI(TAG, "Message sent Failed");
        }
        esp_http_client_cleanup(client);
        vTaskDelay(pdMS_TO_TICKS(600000)); // Make api-request every 10min
    }
}