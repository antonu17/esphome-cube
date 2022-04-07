#include "mic.h"

#include <Arduino.h>
#include <arduinoFFT.h>
#include <driver/i2s.h>
#include <esp32-hal-log.h>
#include <math.h>

#include <string>

#define DELAY pdMS_TO_TICKS(10)

#define N_SAMPLES 1024
#define I2S_READ_LEN (N_SAMPLES * 2)
#define I2S_SAMPLE_RATE (44100)
#define I2S_SAMPLE_BITS (I2S_BITS_PER_SAMPLE_16BIT)
#define I2S_NUM (I2S_NUM_0)

#define I2S_BCK_IO (GPIO_NUM_4)
#define I2S_WS_IO (GPIO_NUM_2)
#define I2S_DO_IO (-1)
#define I2S_DI_IO (GPIO_NUM_15)

static bool initialised = false;
TaskHandle_t i2s_adc_task_handle = NULL;
SemaphoreHandle_t mic_semaphore = NULL;

const uint8_t bars = 8;
const int freqs = N_SAMPLES / 2;

double fft_bars[bars];  // global

void i2s_adc_task(void *);

void i2s_init() {
  ESP_LOGD(__FILE__, "i2s_init(): %d", initialised);

  if (initialised) return;
  initialised = true;

  esp_err_t err;

  mic_semaphore = xSemaphoreCreateBinary();

  const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = I2S_SAMPLE_RATE,
      .bits_per_sample = I2S_SAMPLE_BITS,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 2,
      .dma_buf_len = 1024,
      .use_apll = 1,
  };

  const i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_BCK_IO,
      .ws_io_num = I2S_WS_IO,
      .data_out_num = I2S_DO_IO,
      .data_in_num = I2S_DI_IO,
  };

  err = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    ESP_LOGD(__FILE__, "Failed installing driver: %d\n", err);
  }

  err = i2s_set_pin(I2S_NUM, &pin_config);
  if (err != ESP_OK) {
    ESP_LOGD(__FILE__, "Failed setting pin: %d\n", err);
  }

  err = i2s_set_sample_rates(I2S_NUM, I2S_SAMPLE_RATE);
  if (err != ESP_OK) {
    ESP_LOGD(__FILE__, "Failed setting sample rate: %d\n", err);
  }

  ESP_LOGD(__FILE__, "I2S driver installed.");

  BaseType_t res =
      xTaskCreatePinnedToCore(i2s_adc_task, "i2s_adc_task", 1024 * 64, NULL, 1,
                              &i2s_adc_task_handle, APP_CPU_NUM);
  ESP_LOGD(__FILE__, "I2S read task created with status: %d", res);

  // Initial semaphore kick off
  xSemaphoreGive(mic_semaphore);
}

void i2s_adc_task(void *arg) {
  size_t bytes_read;
  int16_t mic_buffer[N_SAMPLES];

  for (;;) {
    xSemaphoreTake(mic_semaphore, portMAX_DELAY);
    i2s_read(I2S_NUM, (void *)mic_buffer, I2S_READ_LEN, &bytes_read,
             portMAX_DELAY);

    double vReal[N_SAMPLES];
    double vImag[N_SAMPLES];

    for (int i = 0; i < N_SAMPLES; i++) {
      vReal[i] = (double)mic_buffer[i];
      vImag[i] = 0.0;
    }

    arduinoFFT FFT = arduinoFFT(vReal, vImag, N_SAMPLES, I2S_SAMPLE_RATE);
    FFT.DCRemoval();
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(FFT_FORWARD);
    FFT.ComplexToMagnitude();

    // for (uint16_t i = 0; i < N_SAMPLES; i++) {
    //   vReal[i] = 10 * log10((sq(vReal[i]) + sq(vImag[i]))/N_SAMPLES);
    // }

    int f_start = 0;
    for (int i = 0; i < bars; i++) {
      int f_end = sq((float)(i + 1) / bars) * freqs;
      // int f_end = freqs / bars * (i + 1);

      double bar_power = 0.0f;
      double bar_power_avg = 0.0f;

      for (int j = f_start; j < f_end; j++) {
        // double p = 10 * log10((sq(vReal[j]) + sq(vImag[j])) / N_SAMPLES);
        bar_power = fmax(bar_power, vReal[j]);
        bar_power_avg += vReal[j];
      }
      bar_power_avg /= f_end - f_start;

      bar_power_avg = 10 * log10(bar_power_avg);

      float smoothing = 0.4;
      float scale = 0.5;

      fft_bars[i] =
          smoothing * fft_bars[i] + (bar_power_avg * scale * (1.0f - smoothing));

      f_start = f_end;
    }

    xSemaphoreGive(mic_semaphore);
    vTaskDelay(DELAY);
  }
}
