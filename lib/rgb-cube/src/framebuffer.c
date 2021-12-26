#include "framebuffer.h"

#include <Arduino.h>
#include <driver/gpio.h>
#include <esp32-hal-log.h>

#include "tlc59711.h"

DRAM_ATTR rgb_t frame_buffer[512];
static DRAM_ATTR uint8_t rows[] = {AN0, AN1, AN2, AN3, AN4, AN5, AN6, AN7};

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
static bool initialised = false;

void IRAM_ATTR write_row() {
  static DRAM_ATTR uint8_t previous_row = 0, current_row = 0;

  rgb_t *row = frame_buffer + ((7 - current_row) * 64);
  for (int i = 0; i < 64; i++) {
    rgb_t px = *row++;
    tlc_set_led(i, px.r, px.g, px.b);
  }

  gpio_set_level(rows[previous_row], 0);
  tlc_oneshot_write();
  gpio_set_level(rows[current_row], 1);

  previous_row = current_row++;
  current_row &= 0x7;
}

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  write_row();
  portEXIT_CRITICAL_ISR(&timerMux);
}

void init_tlc() {
  tlc_config_t config = {
      .num_drivers = 16,
      .data_pin = 23,
      .sclk_pin = 18,
      .spi_mode = 0,
      .spi_clock_speed_hz = 20000000,  // 20 MHz
  };

  tlc_init(&config);
  tlc_set_brightness(BRIGHTNESS_RED, BRIGHTNESS_GREEN, BRIGHTNESS_BLUE);
}

void init_framebuffer() {
  if (initialised) return;
  initialised = true;

  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;
  gpio_config(&io_conf);

  for (int i = 0; i < 8; i++) {
    gpio_set_level(rows[i], 0);
  }

  init_tlc();
}

void start_framebuffer() {
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  // Update layers every 1250us
  // 8 * 1250us = 10ms (~100 fps)
  timerAlarmWrite(timer, 1250, true);
  timerAlarmEnable(timer);
}

void stop_framebuffer() {
  if (timer) {
    timerEnd(timer);
    timer = NULL;
  }

  for (int i = 0; i < 8; i++) {
    gpio_set_level(rows[i], 0);
  }
}
