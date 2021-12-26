// This file is for debugging purposes

#include "esp32-hal-log.h"

#include <Arduino.h>
#include <WiFi.h>

#include <framebuffer.h>
#include <effect_functions/effect_functions.h>
#include <draw.h>

#define LED_PIN 13

void setup() {

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(xPortGetCoreID());


  ESP_LOGE(TAG, "setup()");

  init_framebuffer();
  sidewaves();
}

void loop() {
}
