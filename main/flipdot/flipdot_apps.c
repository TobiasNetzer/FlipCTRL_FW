#include "flipdot_apps.h"
#include "flipdot_gfx.h"
#include <time.h>
#include <stdio.h>
#include "wifi/openweathermap.h"
#include "bitmaps/bitmaps_weather.h"

void flipdot_app_time(void) {
    static int current_hour = 0, current_min = 0;
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    if(current_hour != timeinfo.tm_hour || current_min != timeinfo.tm_min) {
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

        for(uint8_t i = 0; i < timeinfo.tm_wday * 4; i = i + 4) {
            flipdot_set_pixel(1 + i, 13, WHITE);
            flipdot_set_pixel(2 + i, 13, WHITE);
        }

        flipdot_display();
    }
}

void flipdot_app_weather(void) {
    owm_data_t *owm_data = get_open_weather_data();

    if(!owm_data->updated) return;
    owm_data->updated = false;

    if(owm_data->description_id >= 701 && owm_data->description_id <= 781) owm_data->description_id = 701;
    if(owm_data->description_id >= 600 && owm_data->description_id <= 622) owm_data->description_id = 600;
    if(owm_data->description_id >= 300 && owm_data->description_id <= 531) owm_data->description_id = 300;
    if(owm_data->description_id >= 200 && owm_data->description_id <= 232) owm_data->description_id = 200;

    flipdot_clear();

    char temp_buf[6];
    sprintf(temp_buf, "%d*", owm_data->temperature);

    flipdot_draw_text(12, 3, temp_buf);
    flipdot_draw_line(0, 13, (owm_data->humidity * 28) / 100, 13, WHITE);

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