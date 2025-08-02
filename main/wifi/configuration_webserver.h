#ifndef __CONFIGURATION_WEBSERVER_H__
#define __CONFIGURATION_WEBSERVER_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

#include "esp_http_server.h"

#define MAX_POST_SIZE 512

typedef struct open_weather_config_s{
    char api_key[128];
    char city[64];
    char country_code[16];
} open_weather_config_t;

httpd_handle_t start_configuration_webserver(void);
open_weather_config_t *get_open_weather_config(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __CONFIGURATION_WEBSERVER_H__ */