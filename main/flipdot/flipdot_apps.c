#include "flipdot_apps.h"
#include "flipdot_gfx.h"
#include <time.h>
#include <stdio.h>
#include "wifi/openweathermap.h"
#include "bitmaps/bitmaps_weather.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void flipdot_app_time(bool force_update) {
    static int current_hour = 0, current_min = 0;
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    if(current_hour != timeinfo.tm_hour || current_min != timeinfo.tm_min || force_update) {
        current_hour = timeinfo.tm_hour;
        current_min = timeinfo.tm_min;
        char time_buf[29];
        sprintf(time_buf, "%02d:%02d", current_hour, current_min);

        int ones = 0, current_hour_temp = current_hour;
        while(current_hour_temp > 0) {
            if(current_hour_temp % 10 == 1) {
                ones++;
            }
            current_hour_temp /= 10;
        }

        flipdot_clear();

        flipdot_draw_text(1 + (ones * 2), 3, time_buf);

        int weekday_iso = timeinfo.tm_wday == 0 ? 7 : timeinfo.tm_wday;

        for(uint8_t i = 0; i < weekday_iso * 4; i = i + 4) {
            flipdot_set_pixel(1 + i, 13, WHITE);
            flipdot_set_pixel(2 + i, 13, WHITE);
        }

        flipdot_display();
    }
}

void flipdot_app_weather(bool force_update) {
    owm_data_t *owm_data = get_open_weather_data();

    if(!owm_data->updated && !force_update) return;
    owm_data->updated = false;

    if(owm_data->description_id >= 701 && owm_data->description_id <= 781) owm_data->description_id = 701;
    if(owm_data->description_id >= 600 && owm_data->description_id <= 622) owm_data->description_id = 600;
    if(owm_data->description_id >= 300 && owm_data->description_id <= 531) owm_data->description_id = 300;
    if(owm_data->description_id >= 200 && owm_data->description_id <= 232) owm_data->description_id = 200;

    flipdot_clear();

    char temp_buf[6];
    sprintf(temp_buf, "%d*", owm_data->temperature);

    flipdot_draw_text(12, 3, temp_buf);
    flipdot_draw_line(0, 13, (owm_data->humidity * 27) / 100, 13, WHITE);

    switch(owm_data->description_id) {
        case 200: {
            flipdot_draw_bitmap(2, 2, weather_thunderstorm, WHITE);
        };
        break;
        case 300: {
            flipdot_draw_bitmap(0, 2, weather_rain, WHITE);
        };
        break;
        case 600: {
            flipdot_draw_bitmap(1, 2, weather_snow, WHITE);
        };
        break;
        case 701: {
            flipdot_draw_bitmap(1, 2, weather_mist, WHITE);
        };
        break;
        case 800: {
            if(owm_data->icon_type == 'd') flipdot_draw_bitmap(1, 2, weather_clear_day, WHITE);
            else flipdot_draw_bitmap(1, 2, weather_clear_night, WHITE);
        };
        break;
        case 801: {
            if(owm_data->icon_type == 'd') flipdot_draw_bitmap(0, 2, weather_few_clouds_day, WHITE);
            else flipdot_draw_bitmap(0, 2, weather_few_clouds_night, WHITE);
        };
        break;
        case 802: {
            flipdot_draw_bitmap(0, 3, weather_scattered_clouds, WHITE);
        };
        break;
        case 803: {
            flipdot_draw_bitmap(0, 2, weather_broken_clouds, WHITE);
        };
        break;
        default: {
            flipdot_draw_bitmap(0, 2, weather_broken_clouds, WHITE);
        }
    }

    flipdot_display();
}

void flipdot_app_selector(void *pvParameters) {
    while(1) {
        flipdot_app_t *selected_app = (flipdot_app_t *) pvParameters;

        if (selected_app->app < FLIPDOT_APP_END - 1) (selected_app->app)++;
        else selected_app->app = 0;

        selected_app->force_update = true;

        vTaskDelay(pdMS_TO_TICKS(600000));
    }
}