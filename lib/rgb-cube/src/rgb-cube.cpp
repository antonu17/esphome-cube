#include "rgb-cube.h"

#include <Arduino.h>
#include <Preferences.h>
#include <esp32-hal-log.h>

#include <vector>

#include "draw.h"
#include "effect_functions/effect_functions.h"
#include "effects.h"
#include "framebuffer.h"
#include "tlc59711.h"

std::vector<effect_t *> effects = std::vector<effect_t *>();
static Preferences preferences;

static bool initialized = false, running = false;
static TaskHandle_t effect_loop_task_h, effect_task_h;
static SemaphoreHandle_t effect_task_semaphore, cube_stop_semaphore;
static uint32_t effect_timeout_ms = 0;
static size_t running_effect_idx = 0;
static const char *RUNNING_EFFECT_IDX = "key0";

void effect_loop_task(void *);
void effect_task(void *);

void cube_init() {
  if (initialized) return;
  initialized = true;

  preferences.begin("rgbcube", false);
  running_effect_idx = preferences.getUInt(RUNNING_EFFECT_IDX, 0);
  if (running_effect_idx >= effects.size()) {
    running_effect_idx = effects.size() - 1;
    preferences.putUInt(RUNNING_EFFECT_IDX, running_effect_idx);
  }

  effects.push_back(effect_new("fireworks", fireworks, NULL));
  effects.push_back(effect_new("sidewaves", sidewaves, NULL));
  effects.push_back(effect_new("ball", ball, NULL));
  effects.push_back(effect_new("shift_suspend", shift_planes, NULL));
  effects.push_back(effect_new("cube_color_wheel", cube_color_wheel, NULL));
  effects.push_back(effect_new("linespin", linespin, NULL));
  effects.push_back(effect_new("rain", rain, NULL));
  effects.push_back(effect_new("ripples", ripples, NULL));
  effects.push_back(effect_new("shift_planes", shift_planes, NULL));

  effect_task_semaphore = xSemaphoreCreateBinary();
  cube_stop_semaphore = xSemaphoreCreateBinary();
  init_framebuffer();
}

void cube_start() {
  cube_init();
  if (running) return;
  running = true;

  xTaskCreatePinnedToCore(effect_loop_task, "effect_loop_task", 2000, NULL,
                          tskIDLE_PRIORITY, &effect_loop_task_h, APP_CPU_NUM);
  ESP_LOGD(__FILE__, "effect_loop_task started");

  start_framebuffer();
  ESP_LOGD(__FILE__, "start_framebuffer()");
}

void cube_stop() {
  if (!running) return;
  running = false;

  xSemaphoreGive(effect_task_semaphore);

  // Wait for effect_loop_task to shutdown
  xSemaphoreTake(cube_stop_semaphore, pdMS_TO_TICKS(1000));
  vTaskDelete(effect_loop_task_h);

  fb_clear();
  vTaskDelay(1);
  stop_framebuffer();
  ESP_LOGD(__FILE__, "stop_framebuffer()");
}

void _next_effect() {
  if (running_effect_idx >= (effects.size() - 1))
    running_effect_idx = 0;
  else
    running_effect_idx += 1;
  preferences.putUInt(RUNNING_EFFECT_IDX, running_effect_idx);
}

void _prev_effect() {
  if (running_effect_idx == 0)
    running_effect_idx = (effects.size() - 1);
  else
    running_effect_idx -= 1;
  preferences.putUInt(RUNNING_EFFECT_IDX, running_effect_idx);
}

void cube_next_effect() {
  _next_effect();
  xSemaphoreGive(effect_task_semaphore);
}

void cube_prev_effect() {
  _prev_effect();
  xSemaphoreGive(effect_task_semaphore);
}

void cube_effect_timeout(uint32_t sec) {
  ESP_LOGD(__FILE__, "set effect timeout: %d", sec);
  effect_timeout_ms = pdMS_TO_TICKS(sec * 1000);
}

void cube_set_brightness(uint8_t x) { tlc_set_brightness(x, x, x); }

void effect_loop_task(void *arg) {
  while (running) {
    for (;;) {
      effect_t *effect = effects.at(running_effect_idx);
      ESP_LOGD(__FILE__, "start effect: %s", effect->name);
      xTaskCreatePinnedToCore(effect_task, "effect_task", 4000, effect, 15,
                              &effect_task_h, APP_CPU_NUM);

      uint32_t delta, start = millis();
      for (;;) {
        if (pdTRUE ==
            xSemaphoreTake(effect_task_semaphore, pdMS_TO_TICKS(1000))) {
          break;
        }
        if (effect_timeout_ms == 0) {
          start = millis();
          continue;
        }
        delta = millis() - start;
        if (delta >= effect_timeout_ms) {
          _next_effect();
          break;
        }
      }
      vTaskDelete(effect_task_h);
      if (effect->stop_hook) effect->stop_hook();
      if (!running) break;
    }
  }
  xSemaphoreGive(cube_stop_semaphore);
  vTaskDelay(portMAX_DELAY);
}

void effect_task(void *arg) {
  effect_t *effect = (effect_t *)arg;
  effect->function();
  xSemaphoreGive(effect_task_semaphore);
  vTaskDelay(portMAX_DELAY);
}
