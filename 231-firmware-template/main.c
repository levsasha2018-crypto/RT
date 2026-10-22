#include <stddef.h>

#include "pico/stdlib.h"

#include "api/api.h"
#include "button-task/button-task.h"
#include "firmware.h"
#include "led-task/led-task.h"
#include "logging/log.h"
#include "pi-task/pi-task.h"
#include "profiling/profiling.h"
#include "stdio-text-protocol/stdio-text-protocol.h"

int main(void)
{
    stdio_init_all();
    stdio_text_protocol_init();
    led_task_init();
    button_task_init();
    pi_task_init();
    profiling_init();
    LOG_INF("%s %s\n", DEVICE_NAME, FIRMWARE_VERSION);

    while (1) {
        profiling_iteration();
        button_task_handle();
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL) {
            api_handle(command);
        }
        led_task_handle();
        pi_task_handle();
    }
}
