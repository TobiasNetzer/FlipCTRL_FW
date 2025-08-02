#ifndef __FLIPDOT_APPS_H__
#define __FLIPDOT_APPS_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

#include <stdbool.h>

typedef enum flipdot_apps_e {
    FLIPDOT_TIME,
    FLIPDOT_WEATHER,
    FLIPDOT_APP_END
} flipdot_apps_t;

typedef struct flipdot_app_s {
    bool force_update;
    flipdot_apps_t app;
} flipdot_app_t;

void flipdot_app_time(bool force_update);

void flipdot_app_weather(bool force_update);

void flipdot_app_selector(void *pvParameters);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __FLIPDOT_APPS_H__ */