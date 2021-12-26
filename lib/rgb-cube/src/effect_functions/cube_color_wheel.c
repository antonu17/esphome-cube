#include <Arduino.h>
#include <math.h>

#include "draw.h"

#define SPEED pdMS_TO_TICKS(20)

void cube_color_wheel() {
    rgb_t rgb;
    uint8_t c = 0;

    fb_clear();
    for(;;) {
        rgb = hue_to_rgb_sine2(c);
        for (int px = 0; px < 512; px++) {
            frame_buffer[px] = rgb;
        }
        c++;
        vTaskDelay(SPEED);
    }
    fb_clear();
}
