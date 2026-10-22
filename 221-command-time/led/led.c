#include "led.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "log.h"

static const uint LED_PIN = 25;
static bool state = false;

void led_init(void)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    state = false;
    gpio_put(LED_PIN, state);
}

void led_set(bool enabled)
{
    state = enabled;
    gpio_put(LED_PIN, state);
    LOG_INF("led %s\n", state ? "on" : "off");
}

void led_toggle(void) { led_set(!state); }
bool led_is_on(void) { return state; }
uint led_pin(void) { return LED_PIN; }
