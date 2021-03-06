#include <Arduino.h>
#include <esp32-hal-log.h>
#include <math.h>

#include "draw.h"

#define SPEED pdMS_TO_TICKS(60)
#define DELAY pdMS_TO_TICKS(180)

void sidewaves() {
  float origin_x, origin_y, distance, height, ripple_interval;
  int x, y;
  uint16_t i = 0;

  fb_clear();

  for (;;) {
    i++;
    origin_x = 3.5 + sin((float)i / 500) * 4;
    origin_y = 3.5 + cos((float)i / 500) * 4;

    fb_clear_draft();
    for (x = 0; x < 8; x++) {
      for (y = 0; y < 8; y++) {
        distance = distance2d(origin_x, origin_y, x, y) / 9.899495 * 8;
        ripple_interval = 2;
        height = 4 + sin(distance / ripple_interval + (float)i / 50) * 3.6;

        fb_set_pixel_draft(x, y, (int)height,
                           hue_to_rgb_hsv(150 + ((uint8_t)distance * 24)));
      }
    }
    fb_draw_draft();
    vTaskDelay(1);
  }
  fb_clear();
}
