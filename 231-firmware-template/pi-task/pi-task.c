#include "pi-task/pi-task.h"

#include "pico/stdlib.h"

// 585 cycles per term / 125 MHz * 16 = 74.9 us by the initial estimate.
// A conservative chunk leaves room for interrupts and command handling.
#define PI_TERMS_PER_CALL 16

static pi_state_t pi_state;
static uint32_t pi_k;
static uint32_t pi_terms;
static double pi_sum;
static double pi_sign;
static double pi_value;
static uint64_t started_us;
static uint64_t finished_us;

void pi_task_init(void)
{
    pi_state = PI_STATE_NOT_STARTED;
}

bool pi_task_start(uint32_t terms)
{
    if (pi_state == PI_STATE_RUNNING || terms == 0) {
        return false;
    }
    pi_k = 0;
    pi_terms = terms;
    pi_sum = 0.0;
    pi_sign = 1.0;
    started_us = time_us_64();
    pi_state = PI_STATE_RUNNING;
    return true;
}

void pi_task_handle(void)
{
    if (pi_state != PI_STATE_RUNNING) {
        return;
    }
    // Subtraction avoids k + chunk overflow near UINT32_MAX.
    const uint32_t remaining = pi_terms - pi_k;
    const uint32_t count = remaining < PI_TERMS_PER_CALL ? remaining : PI_TERMS_PER_CALL;
    for (uint32_t i = 0; i < count; i++) {
        pi_sum += pi_sign / (2.0 * pi_k + 1.0);
        pi_sign = -pi_sign;
        pi_k++;
    }
    if (pi_k == pi_terms) {
        pi_value = 4.0 * pi_sum;
        finished_us = time_us_64();
        pi_state = PI_STATE_READY;
    }
}

pi_state_t pi_task_get_result(double *value, uint64_t *elapsed_ms)
{
    if (pi_state == PI_STATE_READY) {
        *value = pi_value;
        *elapsed_ms = (finished_us - started_us) / 1000u;
    }
    return pi_state;
}
