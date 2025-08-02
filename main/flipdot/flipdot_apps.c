#include "flipdot_apps.h"
#include "flipdot_gfx.h"
#include <time.h>
#include <stdio.h>

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
        while (current_hour_temp > 0) {
            if (current_hour_temp % 10 == 1) {
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