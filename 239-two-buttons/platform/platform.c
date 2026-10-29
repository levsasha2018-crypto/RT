#include "platform/platform.h"

#include "pico/stdlib.h"

#define LED_PIN 25
static const uint button_pins[PLATFORM_BUTTON_COUNT] = {15, 14};

void platform_init(void)
{
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    for (uint i = 0; i < PLATFORM_BUTTON_COUNT; i++) {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]);
    }
}

void platform_led_set(bool on) { gpio_put(LED_PIN, on); }
bool platform_button_read(platform_button_t button)
{
    return (unsigned)button < PLATFORM_BUTTON_COUNT && !gpio_get(button_pins[button]);
}
