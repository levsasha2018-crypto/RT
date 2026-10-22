#include "button-task/button-task.h"

#include "pico/stdlib.h"

#include "led-task/led-task.h"

#define BUTTON_PIN 15
#define BUTTON_DEBOUNCE_US 20000

typedef enum {
    BUTTON_RELEASED,
    BUTTON_PRESS_PENDING,
    BUTTON_PRESSED,
    BUTTON_RELEASE_PENDING,
} button_state_t;

static button_state_t button_state;
static uint64_t changed_us;
static uint32_t press_count;

void button_task_init(void)
{
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    button_state = BUTTON_RELEASED;
    changed_us = 0;
    press_count = 0;
}

void button_task_handle(void)
{
    const bool pressed = !gpio_get(BUTTON_PIN);
    const uint64_t now_us = time_us_64();

    switch (button_state) {
    case BUTTON_RELEASED:
        if (pressed) {
            changed_us = now_us;
            button_state = BUTTON_PRESS_PENDING;
        }
        break;
    case BUTTON_PRESS_PENDING:
        if (!pressed) {
            button_state = BUTTON_RELEASED;
        } else if (now_us - changed_us >= BUTTON_DEBOUNCE_US) {
            button_state = BUTTON_PRESSED;
            press_count++;
            led_task_next_state();
        }
        break;
    case BUTTON_PRESSED:
        if (!pressed) {
            changed_us = now_us;
            button_state = BUTTON_RELEASE_PENDING;
        }
        break;
    case BUTTON_RELEASE_PENDING:
        if (pressed) {
            button_state = BUTTON_PRESSED;
        } else if (now_us - changed_us >= BUTTON_DEBOUNCE_US) {
            button_state = BUTTON_RELEASED;
        }
        break;
    }
}

bool button_task_is_pressed(void)
{
    return button_state == BUTTON_PRESSED || button_state == BUTTON_RELEASE_PENDING;
}

uint32_t button_task_get_press_count(void)
{
    return press_count;
}
