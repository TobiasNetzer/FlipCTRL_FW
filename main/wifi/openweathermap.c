#include "openweathermap.h"
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
#include "wifi/configuration_webserver.h"

#define HTTP_RESPONSE_BUFFER_SIZE 1024

static const char *TAG = "HTTP_CLIENT_OPENWEATHERMAP";

static char *response_data = NULL;
static size_t response_len = 0;
static bool all_chunks_received = false;

owm_data_t owm_data = {0};

static esp_err_t get_openweathermap_data(const char *json_string) {
   
    cJSON *root = cJSON_Parse(json_string);

    if(!root) {
    ESP_LOGW(TAG, "Missing Data");
    cJSON_Delete(root);
    free(response_data);
    response_data = NULL;
    response_len = 0;
    return ESP_FAIL;
    }

    cJSON *main = cJSON_GetObjectItem(root, "main");
    if(main) {
        cJSON *temp = cJSON_GetObjectItem(main, "temp");
        cJSON *humidity = cJSON_GetObjectItem(main, "humidity");
        if(cJSON_IsNumber(temp)) {
            owm_data.temperature = temp->valueint;
        }
        if(cJSON_IsNumber(humidity)) {
            owm_data.humidity = humidity->valueint;
        }
    }

    cJSON *weather = cJSON_GetObjectItem(root, "weather");
    if(weather && cJSON_IsArray(weather)) {
        cJSON *weather_item = cJSON_GetArrayItem(weather, 0);
        if(weather_item) {
            cJSON *id = cJSON_GetObjectItem(weather_item, "id");
            cJSON *icon = cJSON_GetObjectItem(weather_item, "icon");
            if(cJSON_IsNumber(id)) {
                owm_data.description_id = id->valueint;
            }
            if(cJSON_IsString(icon)) {
                size_t len = strlen(icon->valuestring);
                owm_data.icon_type = icon->valuestring[len - 1];
            }
        }
    }
    owm_data.updated = true;
    
    cJSON_Delete(root);
    free(response_data);
    response_data = NULL;
    response_len = 0;
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

        size_t url_size = strlen("http://api.openweathermap.org/data/2.5/weather?units=metric&q=") +
                          strlen(open_weather_config->city) +
                          strlen(open_weather_config->country_code) +
                          strlen(open_weather_config->api_key) +
                          strlen(",&appid=") + 1;

        char *open_weather_map_url = malloc(url_size);
        snprintf(open_weather_map_url,
                url_size,
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
        free(open_weather_map_url);
        vTaskDelay(pdMS_TO_TICKS(600000)); // Make api-request every 10min
    }
}

owm_data_t *get_open_weather_data(void) {
    return &owm_data;
}