#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "flipdot_driver.h"
#include "flipdot_gfx.h"

#include "bitmaps.h"

void app_main(void) {
    flipdot_init();
    while(1) {
        flipdot_clear();
        flipdot_draw_bitmap(0,2,weather_rain,true);
        flipdot_draw_text(12,4,"23");
        flipdot_set_pixel(25,5,true);
        flipdot_set_pixel(24,4,true);
        flipdot_set_pixel(26,4,true);
        flipdot_set_pixel(25,3,true);
        flipdot_display();
        vTaskDelay(pdMS_TO_TICKS(3000));
        flipdot_clear();
        flipdot_draw_bitmap(0,2,weather_cloudy,true);
        flipdot_draw_text(12,4,"23");
        flipdot_set_pixel(25,5,true);
        flipdot_set_pixel(24,4,true);
        flipdot_set_pixel(26,4,true);
        flipdot_set_pixel(25,3,true);
        flipdot_display();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
    
}