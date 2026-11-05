#include "api/api-commands.h"
#include "api/api.h"
#include "platform/platform.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "block-task/block-task.h"
#include "stream-task/stream-task.h"

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
    platform_blink_stop();
    (void)command;
    led_task_set_state(LED_STATE_ON);
    print_led();
}

static void cmd_led_disable(const command_t *command)
{
    platform_blink_stop();
    (void)command;
    led_task_set_state(LED_STATE_OFF);
    print_led();
}

static void cmd_led_blink(const command_t *command)
{
    platform_blink_stop();
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
    if (platform_adc_busy()) { printf("error: adc busy\n"); return; }
    (void)command;
    printf("voltage: %.3f V\n", (double)platform_voltage_read());
}

static void cmd_get_temp(const command_t *command)
{
    if (platform_adc_busy()) { printf("error: adc busy\n"); return; }
    (void)command;
    printf("temp: %.1f C\n", (double)platform_temp_read());
}

static void cmd_tm_start(const command_t *command)
{
    if (platform_adc_busy()) { printf("error: adc busy\n"); return; }
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

static bool telemetry_running(void) { return telemetry_task_get_state() == TELEMETRY_STATE_RUN; }

static void cmd_timer_blink_start(const command_t *command)
{
    uint32_t value;
    if (command->argc != 1 || !parse_u32(command->argv[0], &value)) {
        printf("error: usage timer_blink_start <period_ms>\n"); return;
    }
    if (!platform_blink_start(value)) { printf("error: period_ms must be 10..60000\n"); return; }
    led_task_set_state(LED_STATE_OFF);
    printf("timer_blink: started, period %u ms\n", (unsigned)value);
}
static void cmd_timer_blink_stop(const command_t *command)
{
    (void)command;
    printf("timer_blink: stopped, %u toggles\n", (unsigned)platform_blink_stop());
}
static void cmd_cap_start(const command_t *command)
{
    uint32_t count, period;
    if (command->argc != 2 || !parse_u32(command->argv[0], &count) || !parse_u32(command->argv[1], &period)) {
        printf("error: usage cap_start <N> <period_us>\n"); return;
    }
    if (count == 0 || count > PLATFORM_CAPTURE_MAX || period < 10) {
        printf("error: N must be 1..2000, period_us at least 10\n"); return;
    }
    if (telemetry_running() || !platform_capture_start(count, period)) { printf("error: adc busy\n"); return; }
    printf("capture: started, %u samples, period %u us\n", (unsigned)count, (unsigned)period);
}
static void cmd_cap_dump(const command_t *command)
{
    (void)command;
    if (platform_adc_busy()) { printf("error: adc busy\n"); return; }
    if (!platform_capture_done()) { printf("error: no capture\n"); return; }
    platform_capture_t result;
    platform_capture_get(&result);
    printf("capture: %u samples, period %u us, interval %u..%u us, handler %.1f %%\n",
           (unsigned)result.count, (unsigned)result.period_us, (unsigned)result.min_interval_us,
           (unsigned)result.max_interval_us, (double)result.handler_percent);
    for (uint32_t i = 0; i < result.count; i++) {
        if (i % 16 == 0) printf("cap:");
        printf(" %u", (unsigned)result.samples[i]);
        if (i % 16 == 15 || i + 1 == result.count) printf("\n");
    }
}
static void print_dma_cap(const uint16_t *samples, uint32_t count, uint32_t time_us)
{
    uint32_t sum = 0, smallest = 4095, largest = 0;
    for (uint32_t i = 0; i < count; i++) {
        uint32_t value = samples[i];
        sum += value;
        if (value < smallest) smallest = value;
        if (value > largest) largest = value;
    }
    uint32_t expected = (uint64_t)count * 1000000 / PLATFORM_SAMPLE_RATE_HZ;
    printf("dma_cap: %u samples in %u us, expected %u us, min %u max %u mean %.1f\n",
           (unsigned)count, (unsigned)time_us, (unsigned)expected, (unsigned)smallest,
           (unsigned)largest, (double)sum / count);
}
static void start_block(uint32_t count, block_task_callback_t callback)
{
    if (telemetry_running() || !block_task_start(count, callback)) printf("error: adc busy\n");
}
static void cmd_dma_cap(const command_t *command)
{
    uint32_t count;
    if (command->argc != 1 || !parse_u32(command->argv[0], &count)) { printf("error: usage dma_cap <N>\n"); return; }
    if (count == 0 || count > PLATFORM_BLOCK_MAX) { printf("error: N must be 1..8192\n"); return; }
    start_block(count, print_dma_cap);
}
void api_send_frame(const stream_header_t *header, const uint8_t *data, uint32_t bytes)
{
    platform_write(header, sizeof(*header));
    platform_write(data, bytes);
}
static void cmd_stream_start(const command_t *command)
{
    uint32_t bits = 16;
    if (command->argc > 1 || (command->argc == 1 && !parse_u32(command->argv[0], &bits)) || (bits != 8 && bits != 16)) {
        printf("error: usage stream_start [8|16]\n"); return;
    }
    if (telemetry_running() || !stream_task_start((uint8_t)bits)) { printf("error: adc busy\n"); return; }
    printf("stream: started, 44100 Hz, %u bit\n", (unsigned)bits);
    led_task_set_state(LED_STATE_OFF);
    platform_blink_start(250);
}
static void cmd_stream_stop(const command_t *command)
{
    (void)command;
    if (stream_task_get_state() == STREAM_STATE_RUN) { stream_task_stop(); platform_blink_stop(); }
    printf("stream: stopped, %u frames, %u samples, %u overruns\n", (unsigned)stream_task_get_frames(),
           (unsigned)stream_task_get_samples(), (unsigned)stream_task_get_overruns());
}
static void cmd_generate_tone(const command_t *command)
{
    uint32_t hz;
    if (command->argc != 1 || !parse_u32(command->argv[0], &hz)) { printf("error: usage generate_tone <Hz>\n"); return; }
    if (!platform_tone_set(hz)) { printf("error: Hz must be 0 or 20..20000\n"); return; }
    if (hz == 0) printf("tone: off\n"); else printf("tone: %u Hz\n", (unsigned)hz);
}
static void block_stats(const uint16_t *samples, uint32_t count, float *mean, float *rms, float *peak)
{
    uint32_t sum = 0;
    for (uint32_t i = 0; i < count; i++) sum += samples[i];
    int32_t center = (int32_t)(sum / count);
    uint64_t squares = 0;
    uint32_t largest = 0;
    for (uint32_t i = 0; i < count; i++) {
        int32_t deviation = (int32_t)samples[i] - center;
        uint32_t size = deviation < 0 ? -deviation : deviation;
        squares += (uint64_t)(deviation * deviation);
        if (size > largest) largest = size;
    }
    *mean = (float)sum / count;
    *rms = sqrtf((float)squares / count);
    *peak = (float)largest;
}
static void print_mic_bias(const uint16_t *samples, uint32_t count, uint32_t time_us)
{
    (void)time_us;
    uint32_t sum = 0;
    for (uint32_t i = 0; i < count; i++) sum += samples[i];
    printf("mic: bias %.3f V\n", (double)platform_adc_volts((float)sum / count));
}
static void print_mic_level(const uint16_t *samples, uint32_t count, uint32_t time_us)
{
    (void)time_us;
    float mean, rms, peak;
    block_stats(samples, count, &mean, &rms, &peak);
    printf("mic: level %.1f mV rms, peak %.1f mV, %u ms\n", (double)platform_adc_volts(rms) * 1000,
           (double)platform_adc_volts(peak) * 1000, (unsigned)((uint64_t)count * 1000 / PLATFORM_SAMPLE_RATE_HZ));
}
static void cmd_mic_bias(const command_t *command) { (void)command; start_block(4410, print_mic_bias); }
static void cmd_mic_level(const command_t *command)
{
    uint32_t ms = 100;
    if (command->argc > 1 || (command->argc == 1 && !parse_u32(command->argv[0], &ms))) {
        printf("error: usage mic_level [<ms>]\n"); return;
    }
    uint64_t count = (uint64_t)ms * PLATFORM_SAMPLE_RATE_HZ / 1000;
    if (count == 0 || count > PLATFORM_BLOCK_MAX) { printf("error: ms must be 1..185\n"); return; }
    start_block((uint32_t)count, print_mic_level);
}
static void cmd_wdt_enable(const command_t *command)
{
    uint32_t timeout;
    if (command->argc != 1 || !parse_u32(command->argv[0], &timeout)) { printf("error: usage wdt_enable <timeout_ms>\n"); return; }
    if (!platform_watchdog_enable(timeout)) { printf("error: timeout_ms must be 1..8388\n"); return; }
    printf("wdt: enabled, timeout %u ms\n", (unsigned)timeout);
}
static void cmd_hang(const command_t *command)
{
    (void)command;
    printf("hang: superloop stopped\n");
    while (1) { }
}
static void cmd_reset_reason(const command_t *command)
{
    (void)command;
    uint32_t restarts;
    bool watchdog = platform_watchdog_caused_reset(&restarts);
    printf("reset: %s, restarts %u\n", watchdog ? "watchdog" : "other", (unsigned)restarts);
}
const api_command_t api_commands[] = {
    { "timer_blink_start", "timer_blink_start <period_ms>", cmd_timer_blink_start },
    { "timer_blink_stop", "stop timer blink", cmd_timer_blink_stop },
    { "cap_start", "cap_start <N> <period_us>", cmd_cap_start },
    { "cap_dump", "show timer capture", cmd_cap_dump },
    { "dma_cap", "dma_cap <N>", cmd_dma_cap },
    { "stream_start", "stream_start [8|16]", cmd_stream_start },
    { "stream_stop", "stop audio stream", cmd_stream_stop },
    { "generate_tone", "generate_tone <Hz>", cmd_generate_tone },
    { "mic_bias", "microphone DC bias", cmd_mic_bias },
    { "mic_level", "mic_level [<ms>]", cmd_mic_level },
    { "wdt_enable", "wdt_enable <timeout_ms>", cmd_wdt_enable },
    { "hang", "stop the superloop intentionally", cmd_hang },
    { "reset_reason", "last reset and watchdog restart count", cmd_reset_reason },
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
