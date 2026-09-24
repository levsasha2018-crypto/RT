#pragma once
#include <stdbool.h>
#include "pico/types.h"
void led_init(void);
void led_set(bool enabled);
void led_toggle(void);
bool led_is_on(void);
uint led_pin(void);
