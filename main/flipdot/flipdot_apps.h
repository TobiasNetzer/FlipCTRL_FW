#ifndef __FLIPDOT_APPS_H__
#define __FLIPDOT_APPS_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

typedef enum flipdot_app_e {
    FLIPDOT_TIME,
    FLIPDOT_WEATHER
} flipdot_app_t;

void flipdot_app_time(void);

void flipdot_app_weather(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __FLIPDOT_APPS_H__ */