#include <Arduino.h>
#include <math.h>

#include "draw.h"

#define STEP 10

void sinelines() {
  int i, x;
  float left, right, sine_base, x_dividor, ripple_height;
  rgb_t color;
  uint8_t c = 0, step = STEP;

  fb_clear();
  for (i = 0; i < 2000; i++) {
    if (!step--) {
      step = STEP;
      c++;
      color = hue_to_rgb_linear(c);
    }

    fb_clear_draft();
    for (x = 0; x < 8; x++) {
      x_dividor = 2 + sin((float)i / 100) + 1;
      ripple_height = 3 + (sin((float)i / 200) + 1) * 6;

      sine_base = (float)i / 40 + (float)x / x_dividor;

      left = 3.5 + sin(sine_base) * ripple_height;
      right = 3.5 + cos(sine_base) * ripple_height;
      right = 7 - left;

      line_3d(0 - 3, x, (int)left, 7 + 3, x, (int)right, color);
    }
    fb_draw_draft();
    vTaskDelay(5);
  }
  fb_clear();
}
