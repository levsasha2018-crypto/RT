#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "led.h"
#include "log.h"
#include "device.h"

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

void handle_command(int command)
{
    if (command == PICO_ERROR_TIMEOUT) return;
    LOG_DBG("got %c\n", command);
    if (command == 'e') {
        led_set(true);
    } else if (command == 'd') {
        led_set(false);
    } else if (command == 'v') {
        log_version();
    } else if (command == 'i') {
        device_info();
    } else {
        LOG_ERR("unknown command: %c\n", command);
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
        // USB is processed last: an explicit command has priority over the button.
        handle_command(getchar_timeout_us(0));
        sleep_ms(1);
    }
}
