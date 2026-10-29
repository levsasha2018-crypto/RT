#include "led-task/led-task.h"

#include "platform/platform.h"
#include "systime/systime.h"

static led_state_t led_state;
static uint32_t period_ms;
static uint64_t last_us;
static bool led_on;

static void led_set(bool on)
{
    led_on = on;
    platform_led_set(on);
}

void led_task_init(void)
{
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
        last_us = systime_us();
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
        if (systime_period_elapsed(&last_us, (uint64_t)period_ms * 1000u / 2u)) {
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

void led_task_next_period(void)
{
    switch (period_ms) {
    case 250: period_ms = 500; break;
    case 500: period_ms = 1000; break;
    default: period_ms = 250; break;
    }
}

void led_task_blink(void) { led_task_set_state(LED_STATE_BLINK); }
