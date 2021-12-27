#include <Arduino.h>
#include <math.h>

#include "draw.h"

#define STEP 10

void ripples() {
  float distance, height, ripple_interval;
  int x, y;
  uint16_t i = 0;
  uint8_t step = STEP;
  uint8_t c = 150;

  for (;;) {
    i++;

    fb_clear_draft();
    for (x = 0; x < 8; x++) {
      for (y = 0; y < 8; y++) {
        distance = distance2d(3.5, 3.5, x, y);
        ripple_interval = 1.3;
        height = 4 + sin(distance / ripple_interval + (float)i / 50) * 4;
        fb_set_pixel_draft(x, y, (int)height,
                           hue_to_rgb_hsv(c + ((uint8_t)distance * 15)));
      }
    }
    fb_draw_draft();

    if (!step--) {
      step = STEP;
      c++;
    }
    vTaskDelay(1);
  }
}
