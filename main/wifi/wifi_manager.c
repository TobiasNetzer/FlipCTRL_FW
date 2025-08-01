#include "wifi_manager.h"
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
#include <ctype.h>

static const char *TAG = "wifi_manager";

static EventGroupHandle_t wifi_event_group;
static char scanned_ssids[MAX_APS][WIFI_SSID_MAX_LEN];
static uint8_t scanned_count = 0;

char from_hex(char ch) {
    if(isdigit((unsigned char)ch)) return ch - '0';
    if(isalpha((unsigned char)ch)) return tolower((unsigned char)ch) - 'a' + 10;
    return 0;
}

void url_decode(const char *src, char *dest, size_t max_len) {
    size_t i = 0;
    while(*src && i < max_len - 1) {
        if(*src == '%' && isxdigit((unsigned char)src[1]) && isxdigit((unsigned char)src[2])) {
            dest[i++] = (from_hex((unsigned char)src[1]) << 4) |
                         from_hex((unsigned char)src[2]);
            src += 3;
        } else if(*src == '+') {
            dest[i++] = ' ';
            src++;
        } else {
            dest[i++] = *src++;
        }
    }
    dest[i] = '\0';
}

static void save_wifi_creds(char *ssid, char *pass) {
    nvs_handle_t nvs_handle;
    nvs_open("wifi_creds", NVS_READWRITE, &nvs_handle);
    nvs_set_str(nvs_handle, "ssid", ssid);
    nvs_set_str(nvs_handle, "pass", pass);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}

static esp_err_t load_wifi_creds(char *ssid, char *pass) {
    size_t ssid_len = WIFI_SSID_MAX_LEN;
    size_t pass_len = WIFI_PASS_MAX_LEN;
    nvs_handle_t nvs_handle;
    if(nvs_open("wifi_creds", NVS_READONLY, &nvs_handle) == ESP_OK) {
        esp_err_t res1 = nvs_get_str(nvs_handle, "ssid", ssid, &ssid_len);
        esp_err_t res2 = nvs_get_str(nvs_handle, "pass", pass, &pass_len);
        nvs_close(nvs_handle);
        return (res1 == ESP_OK && res2 == ESP_OK);
    }
    return ESP_FAIL;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Retrying...");
        esp_wifi_connect();
    } else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static esp_err_t post_handler(httpd_req_t *req) {
    size_t buf_len = req->content_len;
    char *buf = malloc(buf_len + 1);

    if(buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate buffer for POST data");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal Server Error");
        return ESP_FAIL;
    }

    uint16_t len = httpd_req_recv(req, buf, buf_len);
    if(len <= 0) {
        free(buf);
        ESP_LOGE(TAG, "Failed to receive POST data");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad Request");
        return ESP_FAIL;
    }

    buf[len] = '\0';

    char raw_ssid[WIFI_SSID_MAX_LEN * 3] = {0};
    char raw_pass[WIFI_PASS_MAX_LEN * 3] = {0};

    char ssid[WIFI_SSID_MAX_LEN] = {0};
    char pass[WIFI_PASS_MAX_LEN] = {0};
    
    if(sscanf(buf, "ssid=%31[^&]&pass=%63s", raw_ssid, raw_pass) != 2) {
        free(buf);
        ESP_LOGE(TAG, "Invalid form data received");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid form data");
        return ESP_FAIL;
    }

    url_decode(raw_ssid, ssid, WIFI_SSID_MAX_LEN);
    url_decode(raw_pass, pass, WIFI_PASS_MAX_LEN);

    save_wifi_creds(ssid, pass);

    httpd_resp_sendstr(req, "Credentials saved. Rebooting...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    free(buf);
    esp_restart();
    return ESP_OK;
}

static esp_err_t get_handler(httpd_req_t *req) {
    char html[2048];
    strcpy(html,
        "<html><body><h2>Wi-Fi Setup</h2><form method=\"POST\">"
        "<label>SSID:</label><select name=\"ssid\">");

    for(uint8_t i = 0; i < scanned_count; ++i) {
        strcat(html, "<option>");
        strcat(html, scanned_ssids[i]);
        strcat(html, "</option>");
    }

    strcat(html, "</select><br><label>Password:</label>"
           "<input name=\"pass\" type=\"password\"/><br>"
           "<input type=\"submit\" value=\"Connect\"/></form></body></html>");

    httpd_resp_sendstr(req, html);
    return ESP_OK;
}

static httpd_handle_t start_wifi_setup_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if(httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t get_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = get_handler
        };
        httpd_uri_t post_uri = {
            .uri = "/",
            .method = HTTP_POST,
            .handler = post_handler
        };
        httpd_register_uri_handler(server, &get_uri);
        httpd_register_uri_handler(server, &post_uri);
    }
    return server;
}

static void scan_wifi_networks(void) {
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));

    uint16_t ap_count = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    if (ap_count > MAX_APS) ap_count = MAX_APS;

    wifi_ap_record_t ap_info[MAX_APS];
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_info));
    scanned_count = ap_count;

    for(int i = 0; i < ap_count; ++i) {
        strncpy(scanned_ssids[i], (const char *)ap_info[i].ssid, WIFI_SSID_MAX_LEN - 1);
        scanned_ssids[i][WIFI_SSID_MAX_LEN - 1] = '\0';
        ESP_LOGI(TAG, "Found SSID: %s", scanned_ssids[i]);
    }
}

static void start_ap_mode(void) {
    ESP_LOGI(TAG, "Switching to AP mode...");

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "FlipDot",
            .ssid_len = strlen("FlipDot"),
            .channel = 1,
            .max_connection = 4,
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    start_wifi_setup_webserver();
}

esp_err_t wifi_connect(void) {
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_event_group = xEventGroupCreate();

    char ssid[WIFI_SSID_MAX_LEN] = {0}, pass[WIFI_PASS_MAX_LEN] = {0};
    if(load_wifi_creds(ssid, pass)) {
        ESP_LOGI(TAG, "Found saved Wi-Fi credentials: SSID=%s", ssid);

        esp_netif_create_default_wifi_sta();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

        wifi_config_t wifi_config = {0};
        strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
        strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                               WIFI_CONNECTED_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               pdMS_TO_TICKS(10000));

        if(!(bits & WIFI_CONNECTED_BIT)) {
            ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler));
            ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler));
            esp_wifi_disconnect();
            ESP_LOGW(TAG, "Failed to connect. Scanning before fallback...");
            scan_wifi_networks();
            start_ap_mode();
            return ESP_FAIL;
        }
    } else {
        ESP_LOGI(TAG, "No saved credentials. Scanning before starting AP...");
        esp_netif_create_default_wifi_sta();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
        scan_wifi_networks();
        start_ap_mode();
        return ESP_FAIL;
    }
    return ESP_OK;
}
