#if !defined(EFFECTS_H)
#define EFFECTS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*effect_function_t)();
typedef void (*stop_hook_t)();

typedef struct effect {
    const char *name;
    effect_function_t function;
    stop_hook_t stop_hook;
} effect_t;

effect_t *effect_new(char const *name, effect_function_t function, stop_hook_t stop_hook);
void effect_free(effect_t *effect);

#ifdef __cplusplus
} // extern "C"
#endif

#endif  // EFFECTS_H
