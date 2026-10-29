#include "button-task/button-task.h"

#include <stddef.h>
#include "platform/platform.h"
#include "systime/systime.h"
#define BUTTON_DEBOUNCE_US 20000
#define BUTTON_LONG_PRESS_US 1000000

typedef enum {
    BUTTON_RELEASED,
    BUTTON_PRESS_PENDING,
    BUTTON_PRESSED,
    BUTTON_RELEASE_PENDING,
} button_state_t;

typedef struct {
    button_state_t state;
    uint64_t changed_us;
    uint64_t pressed_us;
    uint32_t press_count;
    bool long_fired;
    button_task_callback_t on_press;
    button_task_callback_t on_long_press;
} button_t;

static button_t buttons[PLATFORM_BUTTON_COUNT];

void button_task_init(platform_button_t button, button_task_callback_t on_press,
                      button_task_callback_t on_long_press)
{
    if ((unsigned)button < PLATFORM_BUTTON_COUNT) {
        buttons[button] = (button_t){ .state = BUTTON_RELEASED,
            .on_press = on_press, .on_long_press = on_long_press };
    }
}

static void handle_button(platform_button_t id, uint64_t now_us)
{
    button_t *b = &buttons[id];
    const bool pressed = platform_button_read(id);

    switch (b->state) {
    case BUTTON_RELEASED:
        if (pressed) {
            b->changed_us = now_us;
            b->state = BUTTON_PRESS_PENDING;
        }
        break;
    case BUTTON_PRESS_PENDING:
        if (!pressed) {
            b->state = BUTTON_RELEASED;
        } else if (now_us - b->changed_us >= BUTTON_DEBOUNCE_US) {
            b->state = BUTTON_PRESSED;
            b->press_count++;
            b->pressed_us = now_us;
            b->long_fired = false;
            if (b->on_long_press == NULL && b->on_press != NULL) {
                b->on_press();
            }
        }
        break;
    case BUTTON_PRESSED:
        if (!pressed) {
            b->changed_us = now_us;
            b->state = BUTTON_RELEASE_PENDING;
        } else if (!b->long_fired && b->on_long_press != NULL &&
                   now_us - b->pressed_us >= BUTTON_LONG_PRESS_US) {
            b->long_fired = true;
            b->on_long_press();
        }
        break;
    case BUTTON_RELEASE_PENDING:
        if (pressed) {
            b->state = BUTTON_PRESSED;
        } else if (now_us - b->changed_us >= BUTTON_DEBOUNCE_US) {
            b->state = BUTTON_RELEASED;
            if (b->on_long_press != NULL && !b->long_fired && b->on_press != NULL) {
                b->on_press();
            }
        }
        break;
    }
}

void button_task_handle(void)
{
    const uint64_t now_us = systime_us();
    for (uint32_t id = 0; id < PLATFORM_BUTTON_COUNT; id++) {
        handle_button((platform_button_t)id, now_us);
    }
}

bool button_task_is_pressed(platform_button_t button)
{
    if ((unsigned)button >= PLATFORM_BUTTON_COUNT) return false;
    return buttons[button].state == BUTTON_PRESSED || buttons[button].state == BUTTON_RELEASE_PENDING;
}

uint32_t button_task_get_press_count(platform_button_t button)
{
    return (unsigned)button < PLATFORM_BUTTON_COUNT ? buttons[button].press_count : 0;
}
