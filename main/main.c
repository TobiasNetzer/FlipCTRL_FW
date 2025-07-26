#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "flipdot_driver.h"
#include "flipdot_gfx.h"

#include "bitmaps/bitmaps_weather.h"
#include "wifi/wifi_manager.h"

void app_main(void) {

    wifi_connect();

    flipdot_init();

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