#if !defined(RGBCUBE_H)
#define RGBCUBE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void cube_init();
void cube_start();
void cube_stop();
void cube_effect_timeout(uint32_t);
void cube_set_brightness(uint8_t);
void cube_next_effect();
void cube_prev_effect();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // RGBCUBE_H
