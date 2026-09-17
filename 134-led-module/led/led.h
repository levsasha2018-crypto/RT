#pragma once
#include <stdbool.h>
void led_init(void);
void led_set(bool enabled);
void led_toggle(void);
bool led_is_on(void);
