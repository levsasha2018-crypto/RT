#pragma once

#include <stdbool.h>

void platform_init(void);
void platform_led_set(bool on);
typedef enum {
    PLATFORM_BUTTON_LED,
    PLATFORM_BUTTON_PERIOD,
    PLATFORM_BUTTON_COUNT,
} platform_button_t;
bool platform_button_read(platform_button_t button);
