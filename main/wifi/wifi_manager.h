#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

#include <stdbool.h>
#include "esp_system.h"

#define WIFI_SSID_MAX_LEN 32
#define WIFI_PASS_MAX_LEN 64
#define MAX_APS 10

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

esp_err_t wifi_connect(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __WIFI_MANAGER_H__ */