#include <Arduino.h>

#include "draw.h"
#include "mic.h"
#include "effect_functions.h"

#define SPEED pdMS_TO_TICKS(10)

void mic_spectrum() {
  i2s_init();

  for (;;) {
    xSemaphoreTake(mic_semaphore, portMAX_DELAY);
    fb_shift(FB_AXIS_X, FB_SHIFT_BACK);
    uint8_t c = 0;

    for (int x = 0; x < 8; x++) {
      uint8_t v = fft_bars[x] / 20.0 * 8;
      rgb_t rgb = hue_to_rgb_sine2(c);
      fb_set_pixel(7, x, v, rgb);
      c += 32;
    }
    xSemaphoreGive(mic_semaphore);
    vTaskDelay(SPEED);
  }
}
