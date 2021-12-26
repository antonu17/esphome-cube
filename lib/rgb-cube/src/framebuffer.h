#if !defined(FRAMEBUFFER_H)
#define FRAMEBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "colors.h"

extern rgb_t frame_buffer[512];

#define AN0 32
#define AN1 33
#define AN2 25
#define AN3 26
#define AN4 27
#define AN5 14
#define AN6 12
#define AN7 13

#define GPIO_OUTPUT_PIN_SEL ((1ULL << AN0) | (1ULL << AN1) | (1ULL << AN2) | (1ULL << AN3) | (1ULL << AN4) | (1ULL << AN5) | (1ULL << AN6) | (1ULL << AN7))

#define BRIGHTNESS_RED 180
#define BRIGHTNESS_GREEN 180
#define BRIGHTNESS_BLUE 180

void init_framebuffer();
void start_framebuffer();
void stop_framebuffer();

#ifdef __cplusplus
} // extern "C"
#endif

#endif  // FRAMEBUFFER_H
