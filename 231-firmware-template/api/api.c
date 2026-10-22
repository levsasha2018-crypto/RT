#include "api/api.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include "button-task/button-task.h"
#include "device/device.h"
#include "led-task/led-task.h"
#include "pi-task/pi-task.h"
#include "profiling/profiling.h"

typedef struct
{
    const char *name;
    const char *help;
    void (*callback)(const command_t *command);
} api_command_t;

static bool parse_u32(const char *text, uint32_t *value)
{
    if (text == NULL || *text == '\0') {
        return false;
    }
    for (const char *p = text; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            return false;
        }
    }
    errno = 0;
    char *end;
    const unsigned long number = strtoul(text, &end, 10);
    if (errno == ERANGE || *end != '\0' || number > UINT32_MAX) {
        return false;
    }
    *value = (uint32_t)number;
    return true;
}

static void cmd_info(const command_t *command)
{
    (void)command;
    device_info();
}

static void cmd_uptime(const command_t *command)
{
    (void)command;
    printf("uptime: %llu ms\n", (unsigned long long)(time_us_64() / 1000u));
}

static void print_led(void)
{
    const char *name = "off";
    switch (led_task_get_state()) {
    case LED_STATE_OFF: break;
    case LED_STATE_ON: name = "on"; break;
    case LED_STATE_BLINK: name = "blink"; break;
    }
    printf("led: %s, period %u ms\n", name, (unsigned)led_task_get_period_ms());
}

static void cmd_led_enable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_ON);
    print_led();
}

static void cmd_led_disable(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_OFF);
    print_led();
}

static void cmd_led_blink(const command_t *command)
{
    (void)command;
    led_task_set_state(LED_STATE_BLINK);
    print_led();
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;
    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms)) {
        printf("error: usage led_period <period_ms>\n");
        return;
    }
    if (!led_task_set_period_ms(period_ms)) {
        printf("error: period_ms must be greater than 0\n");
        return;
    }
    print_led();
}

static void cmd_button(const command_t *command)
{
    (void)command;
    printf("button: %s, presses %u\n",
           button_task_is_pressed() ? "pressed" : "released",
           (unsigned)button_task_get_press_count());
}

static void cmd_pi_start(const command_t *command)
{
    uint32_t terms = 1000000;
    if (command->argc > 1 ||
        (command->argc == 1 && !parse_u32(command->argv[0], &terms))) {
        printf("error: usage pi_start [<terms>]\n");
        return;
    }
    if (terms == 0) {
        printf("error: terms must be greater than 0\n");
        return;
    }
    if (!pi_task_start(terms)) {
        printf("error: pi is busy\n");
        return;
    }
    printf("pi: started, %u terms\n", (unsigned)terms);
}

static void cmd_pi(const command_t *command)
{
    (void)command;
    double value;
    uint64_t elapsed_ms;
    switch (pi_task_get_result(&value, &elapsed_ms)) {
    case PI_STATE_NOT_STARTED:
        printf("pi: not started\n");
        break;
    case PI_STATE_RUNNING:
        printf("pi: running\n");
        break;
    case PI_STATE_READY:
        printf("pi: %.9f in %llu ms\n", value, (unsigned long long)elapsed_ms);
        break;
    }
}

static void cmd_main_time_exec(const command_t *command)
{
    (void)command;
    printf("iteration avg %.2f us, max %u us\n",
           (double)profiling_avg_us(), (unsigned)profiling_max_us());
}

static void cmd_main_time_reset(const command_t *command)
{
    (void)command;
    profiling_reset_max();
    printf("max reset\n");
}

static const api_command_t commands[] = {
    { "info", "device passport", cmd_info },
    { "uptime", "time since reset", cmd_uptime },
    { "led_enable", "turn LED on", cmd_led_enable },
    { "led_disable", "turn LED off", cmd_led_disable },
    { "led_blink", "blink LED", cmd_led_blink },
    { "led_period", "set blink period: led_period <period_ms>", cmd_led_period },
    { "button", "button state and press count", cmd_button },
    { "pi_start", "start pi calculation: pi_start [<terms>]", cmd_pi_start },
    { "pi", "pi calculation state or last result", cmd_pi },
    { "main_time_exec", "superloop iteration: average and maximum", cmd_main_time_exec },
    { "main_time_reset", "reset maximum iteration", cmd_main_time_reset },
};

void api_handle(const command_t *command)
{
    if (command == NULL) {
        return;
    }
    if (command->truncated) {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }
    if (command->name == NULL) {
        return;
    }
    if (strcmp(command->name, "help") == 0) {
        printf("%-17s %s\n", "help", "list of commands");
        for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
            printf("%-17s %s\n", commands[i].name, commands[i].help);
        }
        return;
    }
    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if (strcmp(command->name, commands[i].name) == 0) {
            commands[i].callback(command);
            return;
        }
    }
    printf("error: unknown command '%s', try help\n", command->name);
}
