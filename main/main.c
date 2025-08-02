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

    flipdot_app_t flipdot_app = FLIPDOT_TIME;

    flipdot_init();

    if(wifi_connect() == ESP_OK) {
        start_configuration_webserver();
        start_sntp_time_sync();
		xTaskCreate(&openweather_api_http, "openweather_api_http", 8192, NULL, 6, NULL);
	} else {
        flipdot_draw_line(9, 2, 18,11, WHITE);
        flipdot_draw_line(9, 11, 18,2, WHITE);
        flipdot_display();
        while(1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    while(1) {
        switch(flipdot_app) {
            case FLIPDOT_TIME: {
                flipdot_app_time();
            };
            break;
            case FLIPDOT_WEATHER: {
                flipdot_app_time();
            };
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    const uint16_t* test_bitmaps[] = {weather_clear_day, weather_clear_night, weather_few_clouds_day, weather_few_clouds_night, weather_scattered_clouds, weather_broken_clouds, weather_rain, weather_thunderstorm, weather_snow, weather_mist};
    while(1) {

        for(uint8_t i = 0; i<10; i++) {
            flipdot_clear();
            flipdot_draw_bitmap(0,2,test_bitmaps[i],true);
            flipdot_draw_text(12,4,"23");
            flipdot_set_pixel(25,5,true);
            flipdot_set_pixel(24,4,true);
            flipdot_set_pixel(26,4,true);
            flipdot_set_pixel(25,3,true);
            flipdot_display();
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }
}