#include "api/api-commands.h"
#include "api/api.h"
#include "platform/platform.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "firmware.h"
#include "systime/systime.h"
#include "button-task/button-task.h"
#include "device/device.h"
#include "led-task/led-task.h"
#include "pi-task/pi-task.h"
#include "profiling/profiling.h"

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
    device_info_t info;
    device_get_info(&info);
    printf("name: %s\n", FIRMWARE_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);
    printf("project: %s\n", FIRMWARE_PROJECT);
    printf("repo: %s\n", FIRMWARE_REPO);
    printf("board: %s\n", info.board);
    printf("serial: %s\n", info.serial);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n",
           (unsigned)info.manufacturer, (unsigned)info.part, (unsigned)info.revision);
    printf("pico-sdk: %s\n", info.sdk_version);
}

static void cmd_uptime(const command_t *command)
{
    (void)command;
    printf("uptime: %llu ms\n", (unsigned long long)systime_ms());
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

static void cmd_profiling(const command_t *command)
{
    (void)command;
#if PROFILING_ENABLED
    profiling_result_t result;
    for (uint32_t id = 0; id < PROFILING_STOPWATCH_COUNT; id++) {
        if (profiling_get(id, &result)) {
            printf("%-10s count %-10u mean %.2f us  max %u us\n",
                   result.name, (unsigned)result.count,
                   (double)result.mean_us, (unsigned)result.max_us);
        }
    }
#else
    printf("profiling is disabled in this build\n");
#endif
}

static void cmd_profiling_reset(const command_t *command)
{
    (void)command;
    profiling_reset();
    printf("profiling reset\n");
}

void api_print_telemetry(const telemetry_record_t *record)
{
    printf("tm: %llu %.3f %.1f\n", (unsigned long long)record->time_us,
           (double)record->voltage_v, (double)record->temp_c);
}

static void cmd_get_voltage(const command_t *command)
{
    (void)command;
    printf("voltage: %.3f V\n", (double)platform_voltage_read());
}

static void cmd_get_temp(const command_t *command)
{
    (void)command;
    printf("temp: %.1f C\n", (double)platform_temp_read());
}

static void cmd_tm_start(const command_t *command)
{
    uint32_t period = 100;
    if (command->argc > 1 || (command->argc == 1 && !parse_u32(command->argv[0], &period))) {
        printf("error: usage tm_start [<period_ms>]\n");
    } else if (!telemetry_task_start(period)) {
        printf("error: period_ms must be at least 10\n");
    } else {
        printf("telemetry: started, period %u ms\n", (unsigned)period);
    }
}

static void cmd_tm_stop(const command_t *command)
{
    (void)command;
    telemetry_task_stop();
    printf("telemetry: stopped\n");
}

static void cmd_charge_on(const command_t *command)
{
    (void)command;
    platform_charge_set(true);
    printf("charge: on\n");
}

static void cmd_charge_off(const command_t *command)
{
    (void)command;
    platform_charge_set(false);
    printf("charge: off\n");
}

const api_command_t api_commands[] = {
    { "get_voltage", "voltage on GP26", cmd_get_voltage },
    { "get_temp", "chip temperature in Celsius", cmd_get_temp },
    { "tm_start", "start telemetry: tm_start [<period_ms>]", cmd_tm_start },
    { "tm_stop", "stop telemetry", cmd_tm_stop },
    { "charge_on", "charge RC circuit through GP22", cmd_charge_on },
    { "charge_off", "discharge RC circuit through GP22", cmd_charge_off },
    { "info", "device passport", cmd_info },
    { "uptime", "time since reset", cmd_uptime },
    { "led_enable", "turn LED on", cmd_led_enable },
    { "led_disable", "turn LED off", cmd_led_disable },
    { "led_blink", "blink LED", cmd_led_blink },
    { "led_period", "set blink period: led_period <period_ms>", cmd_led_period },
    { "button", "button state and press count", cmd_button },
    { "pi_start", "start pi calculation: pi_start [<terms>]", cmd_pi_start },
    { "pi", "pi calculation state or last result", cmd_pi },
    { "profiling", "stopwatch counts, means and maxima", cmd_profiling },
    { "profiling_reset", "reset all stopwatches", cmd_profiling_reset },
};

const uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);
