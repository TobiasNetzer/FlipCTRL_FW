#ifndef __OPENWEATHERMAP_H__
#define __OPENWEATHERMAP_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

#include <stdio.h>
#include <stdbool.h>

typedef struct owm_data_s {
    int8_t temperature;
    uint8_t humidity;
    uint16_t description_id;
    char icon_type;
    bool updated;
} owm_data_t;

void openweather_api_http(void *pvParameters);

owm_data_t *get_open_weather_data(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __OPENWEATHERMAP_H__ */