#include "led-task/led-task.h"

#include "pico/stdlib.h"

#define LED_PIN 25

static led_state_t led_state;
static uint32_t period_ms;
static uint64_t last_us;
static bool led_on;

static void led_set(bool on)
{
    led_on = on;
    gpio_put(LED_PIN, on);
}

static bool period_elapsed(uint64_t *previous_us, uint64_t period_us)
{
    const uint64_t now_us = time_us_64();
    if (now_us - *previous_us < period_us) {
        return false;
    }
    *previous_us += period_us;
    if (now_us - *previous_us >= period_us) {
        *previous_us = now_us;
    }
    return true;
}

void led_task_init(void)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    led_on = false;
    period_ms = 1000;
    led_task_set_state(LED_STATE_BLINK);
}

void led_task_set_state(led_state_t state)
{
    led_state = state;
    switch (state) {
    case LED_STATE_OFF:
        led_set(false);
        break;
    case LED_STATE_ON:
        led_set(true);
        break;
    case LED_STATE_BLINK:
        led_set(!led_on);
        last_us = time_us_64();
        break;
    }
}

void led_task_handle(void)
{
    switch (led_state) {
    case LED_STATE_OFF:
    case LED_STATE_ON:
        break;
    case LED_STATE_BLINK:
        // Convert before multiplying: UINT32_MAX milliseconds must fit as well.
        if (period_elapsed(&last_us, (uint64_t)period_ms * 1000u / 2u)) {
            led_set(!led_on);
        }
        break;
    }
}

led_state_t led_task_get_state(void)
{
    return led_state;
}

bool led_task_set_period_ms(uint32_t value_ms)
{
    if (value_ms == 0) {
        return false;
    }
    period_ms = value_ms;
    return true;
}

uint32_t led_task_get_period_ms(void)
{
    return period_ms;
}

void led_task_next_state(void)
{
    switch (led_state) {
    case LED_STATE_OFF: led_task_set_state(LED_STATE_ON); break;
    case LED_STATE_ON: led_task_set_state(LED_STATE_BLINK); break;
    case LED_STATE_BLINK: led_task_set_state(LED_STATE_OFF); break;
    }
}
