#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const uint LED_PIN = 25;
const uint BUTTON_PIN = 15;

// One button, active low. Accept both edges only after 20 ms without changes.
// A monotonic timer keeps USB command processing responsive during debounce.
const uint DEBOUNCE_MS = 20;

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

static void set_led(bool enabled)
{
    gpio_put(LED_PIN, enabled);
    printf("led %s\n", enabled ? "on" : "off");
}

bool handle_command(int command, bool led)
{
    if (command == PICO_ERROR_TIMEOUT) return led;
    if (command == 'e') {
        led = true;
        set_led(led);
    } else if (command == 'd') {
        led = false;
        set_led(led);
    } else {
        printf("unknown command: %c\n", command);
    }
    return led;
}

int main(void)
{
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    bool led = false;
    bool previous = true;
    while (1) {
        const bool current = get_button_debounce(BUTTON_PIN);
        if (previous && !current) {
            led = !led;
            set_led(led);
        }
        previous = current;
        // USB wins when a button edge and a command occur in the same iteration.
        const int command = getchar_timeout_us(0);
        led = handle_command(command, led);
        sleep_ms(1);
    }
}
