#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "command.h"
#include "device.h"
#include "led.h"
#include "log.h"
#include "memory.h"

#define LINE_SIZE 32

const uint BUTTON_PIN = 15;
const uint DEBOUNCE_MS = 20;

char line[LINE_SIZE];
uint line_length = 0;

static void cmd_enable(void) { led_set(true); }
static void cmd_disable(void) { led_set(false); }
static void cmd_info(void) { device_info(); }
static void cmd_version(void) { log_version(); }
static void cmd_ping(void) { printf("pong\n"); }
static void cmd_mem_info(void) { mem_info(); }
static void cmd_fw_info(void) { fw_info(); }
static void cmd_dev_info(void) { dev_info(); }
static void cmd_boot_info(void) { boot_info(); }

const struct command_t commands[] = {
    { "enable", cmd_enable },
    { "disable", cmd_disable },
    { "info", cmd_info },
    { "version", cmd_version },
    { "ping", cmd_ping },
    { "mem_info", cmd_mem_info },
    { "fw_info", cmd_fw_info },
    { "dev_info", cmd_dev_info },
    { "boot_info", cmd_boot_info },
};

const uint command_count = sizeof(commands) / sizeof(commands[0]);

bool get_button_debounce(uint pin)
{
    static bool candidate = true;
    static bool stable = true;
    static uint64_t changed_at = 0;
    const bool sample = gpio_get(pin);
    const uint64_t now = time_us_64();

    if (sample != candidate) {
        candidate = sample;
        changed_at = now;
    }
    if (now - changed_at >= (uint64_t)DEBOUNCE_MS * 1000u) {
        stable = candidate;
    }
    return stable;
}

void handle_command(const char *command)
{
    for (uint i = 0; i < command_count; i++) {
        if (strcmp(command, commands[i].name) == 0) {
            if (commands[i].handler != NULL) {
                commands[i].handler();
            }
            return;
        }
    }

    LOG_ERR("unknown command: %s\n", command);
}

void read_line(void)
{
    int symbol = getchar_timeout_us(0);

    if (symbol == PICO_ERROR_TIMEOUT) {
        return;
    }

    if (symbol == '\r' || symbol == '\n') {
        putchar('\n');
        line[line_length] = '\0';
        if (line_length > 0) {
            LOG_DBG("got %s\n", line);
            handle_command(line);
        }
        line_length = 0;
        return;
    }

    if (line_length + 1 < LINE_SIZE) {
        line[line_length] = (char)symbol;
        line_length++;
        putchar(symbol);
    }
}

int main(void)
{
    stdio_init_all();
    led_init();
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    bool previous = true;
    while (1) {
        const bool current = get_button_debounce(BUTTON_PIN);
        if (previous && !current) {
            led_toggle();
        }
        previous = current;
        read_line();
        sleep_ms(1);
    }
}
