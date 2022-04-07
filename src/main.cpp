// This file is for debugging purposes

#include <Arduino.h>

#include "esp32-hal-log.h"

TaskHandle_t xTask = NULL;
SemaphoreHandle_t xSemaphore = NULL;
int counter = 0;

void counterTask(void *arg) {
  for (;;) {
    Serial.printf("in counter loop()\n");
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    counter++;
    xSemaphoreGive(xSemaphore);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(115200);
  xSemaphore = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(counterTask, "counterTask", 4000, NULL,
                          tskIDLE_PRIORITY, &xTask, APP_CPU_NUM);
  xSemaphoreGive(xSemaphore);
  Serial.printf("setup()\n");
}

void loop() {
  Serial.printf("enter loop()\n");
  xSemaphoreTake(xSemaphore, portMAX_DELAY);
  Serial.printf("counter: %d\n", counter);
  xSemaphoreGive(xSemaphore);
  vTaskDelay(pdMS_TO_TICKS(1000));
  Serial.printf("exit loop()\n");
}
