#if !defined(MIC_H)
#define MIC_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

extern SemaphoreHandle_t mic_semaphore;
extern double fft_bars[];

void i2s_init();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // MIC_H
