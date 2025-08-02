#include "configuration_webserver.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"
#include "esp_http_server.h"

static const char *TAG = "OWM_CGF";

static open_weather_config_t current_config = {0};

void load_open_weather_config_from_nvs(void) {
    nvs_handle_t nvs;
    size_t len;
    if(nvs_open("storage", NVS_READONLY, &nvs) == ESP_OK) {
        len = sizeof(current_config.api_key);
        nvs_get_str(nvs, "api_key", current_config.api_key, &len);

        len = sizeof(current_config.city);
        nvs_get_str(nvs, "city", current_config.city, &len);

        len = sizeof(current_config.country_code);
        nvs_get_str(nvs, "country_code", current_config.country_code, &len);

        nvs_close(nvs);
        ESP_LOGI(TAG, "Loaded config from NVS");
    } else {
        ESP_LOGW(TAG, "No config found in NVS");
    }
}

void save_config_to_nvs_and_memory(const char *api_key, const char *city, const char *country_code) {
    nvs_handle_t nvs;
    if(nvs_open("storage", NVS_READWRITE, &nvs) == ESP_OK) {
        nvs_set_str(nvs, "api_key", api_key);
        nvs_set_str(nvs, "city", city);
        nvs_set_str(nvs, "country_code", country_code);
        nvs_commit(nvs);
        nvs_close(nvs);
        ESP_LOGI(TAG, "Saved to NVS");
    }

    strncpy(current_config.api_key, api_key, sizeof(current_config.api_key));
    strncpy(current_config.city, city, sizeof(current_config.city));
    strncpy(current_config.country_code, country_code, sizeof(current_config.country_code));
}

esp_err_t get_handler(httpd_req_t *req) {
    char html[1024];
    snprintf(html, sizeof(html),
        "<!DOCTYPE html><html><head><title>OpenWeatherMap Config</title></head><body>"
        "<h2>Configure OpenWeatherMap</h2>"
        "<form action=\"/submit\" method=\"POST\">"
        "<label>API Key:</label><br>"
        "<input type=\"text\" name=\"api_key\" value=\"%s\"><br><br>"
        "<label>City:</label><br>"
        "<input type=\"text\" name=\"city\" value=\"%s\"><br><br>"
        "<label>Country Code:</label><br>"
        "<input type=\"text\" name=\"country_code\" value=\"%s\"><br><br>"
        "<input type=\"submit\" value=\"Save\">"
        "</form></body></html>",
        current_config.api_key,
        current_config.city,
        current_config.country_code
    );
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t post_handler(httpd_req_t *req) {
    char buf[MAX_POST_SIZE] = {0};

    int received = httpd_req_recv(req, buf, MAX_POST_SIZE - 1);
    if(received <= 0) {
        ESP_LOGE(TAG, "Failed to receive POST data");
        return ESP_FAIL;
    }

    char *api_key = NULL, *city = NULL, *country_code = NULL;

    char *token = strtok(buf, "&");
    while (token) {
        if(strncmp(token, "api_key=", 8) == 0) {
            api_key = token + 8;
        } else if(strncmp(token, "city=", 5) == 0) {
            city = token + 5;
        } else if(strncmp(token, "country_code=", 13) == 0) {
            country_code = token + 13;
        }
        token = strtok(NULL, "&");
    }

    if(api_key && city && country_code) {
        save_config_to_nvs_and_memory(api_key, city, country_code);
        httpd_resp_sendstr(req, "Settings saved successfully. <a href=\"/\">Return</a>");
    } else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing fields");
    }

    return ESP_OK;
}

httpd_handle_t start_configuration_webserver(void) {

    load_open_weather_config_from_nvs();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if(httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t form = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = get_handler,
            .user_ctx = NULL
        };
        httpd_uri_t submit = {
            .uri = "/submit",
            .method = HTTP_POST,
            .handler = post_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &form);
        httpd_register_uri_handler(server, &submit);
    }
    return server;
}

open_weather_config_t *get_open_weather_config(void) {
    return &current_config;
}