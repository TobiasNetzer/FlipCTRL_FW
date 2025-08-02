#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>
#include "esp_log.h"

#include "flipdot/flipdot_driver.h"
#include "flipdot/flipdot_gfx.h"
#include "flipdot/flipdot_apps.h"

#include "bitmaps/bitmaps_weather.h"
#include "wifi/wifi_manager.h"

#include "wifi/openweathermap.h"
#include "wifi/configuration_webserver.h"
#include "wifi/sntp.h"

void app_main(void) {

    flipdot_app_t flipdot_app = {.app = FLIPDOT_APP_END,
                                .force_update = true };

    flipdot_init();

    if(wifi_connect() == ESP_OK) {
        start_configuration_webserver();
        start_sntp_time_sync();
		xTaskCreate(&openweather_api_http, "openweather_api_http", 8192, NULL, 6, NULL);
        xTaskCreate(&flipdot_app_selector, "flipdot_app_selector", 8192, &flipdot_app, 5, NULL);
        vTaskDelay(pdMS_TO_TICKS(1000));
	} else {
        flipdot_draw_line(9, 2, 18,11, WHITE);
        flipdot_draw_line(9, 11, 18,2, WHITE);
        flipdot_display();
        while(1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    while(1) {
        switch(flipdot_app.app) {
            case FLIPDOT_TIME: {
                flipdot_app_time(flipdot_app.force_update);
            };
            break;
            case FLIPDOT_WEATHER: {
                flipdot_app_weather(flipdot_app.force_update);
            };
            break;
            default: break;
        }
        flipdot_app.force_update = false;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}