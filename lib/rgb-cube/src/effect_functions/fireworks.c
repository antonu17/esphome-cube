#include <Arduino.h>
#include <math.h>

#include "draw.h"

#define SPEED pdMS_TO_TICKS(80)
#define DELAY pdMS_TO_TICKS(800)

void fireworks() {
  uint8_t n = 50;

  fb_clear();
  for (;;) {
    fb_clear();

    uint8_t f, e;

    float origin_x = 3;
    float origin_y = 3;
    float origin_z = 3;

    uint8_t rand_y, rand_x, rand_z;

    float slowrate, gravity;

    // Particles and their position, x,y,z,
    // their movement, dx, dy, dz
    float particles[n][6];
    rgb_t particles_color[n];

    origin_x = rand() % 4;
    origin_y = rand() % 4;
    origin_z = rand() % 2;
    origin_z += 5;
    origin_x += 2;
    origin_y += 2;

    rgb_t color = hue_to_rgb_linear(rand() % 255);
    bool random_colors = (rand() % 2) == 1;

    // shoot a particle up in the air
    for (e = 0; e < origin_z; e++) {
      fb_set_pixel(origin_x, origin_y, e, WHITE);
      vTaskDelay(pdMS_TO_TICKS(60 + 50 * e));
      fb_clear();
    }

    // Fill particle array
    for (f = 0; f < n; f++) {
      // Position
      particles[f][0] = origin_x;
      particles[f][1] = origin_y;
      particles[f][2] = origin_z;

      rand_x = rand() % 200;
      rand_y = rand() % 200;
      rand_z = rand() % 200;

      // Movement
      particles[f][3] = 1 - (float)rand_x / 100;  // dx
      particles[f][4] = 1 - (float)rand_y / 100;  // dy
      particles[f][5] = 1 - (float)rand_z / 100;  // dz

      // Color
      if (random_colors)
        particles_color[f] = hue_to_rgb_linear(rand() % 255);
      else
        particles_color[f] = color;
    }

    // explode
    for (e = 0; e < 25; e++) {
      slowrate = 1 + tan((e + 0.1) / 20) * 10;
      gravity = tan((e + 0.1) / 20) / 2;

      for (f = 0; f < n; f++) {
        particles[f][0] += particles[f][3] / slowrate;
        particles[f][1] += particles[f][4] / slowrate;
        particles[f][2] += particles[f][5] / slowrate;
        particles[f][2] -= gravity;

        rgb_t c = particles_color[f];
        // rgb_t c = hue_to_rgb_linear(rand() % 255);

        fb_set_pixel(particles[f][0], particles[f][1], particles[f][2], c);
      }

      vTaskDelay(SPEED);
      fb_clear();
    }
  }
  fb_clear();
}
