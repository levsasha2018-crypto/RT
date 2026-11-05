#include "stream-task/stream-task.h"
#include "platform/platform.h"
#include <stddef.h>
_Static_assert(sizeof(stream_header_t) == 6, "Stream header must be six bytes");
static stream_task_callback_t callback;
static stream_state_t state;
static uint8_t stream_bits;
static uint16_t number;
static uint32_t frames, samples;
void stream_task_init(stream_task_callback_t on_frame) { callback = on_frame; state = STREAM_STATE_IDLE; }
bool stream_task_start(uint8_t bits)
{
    if (state == STREAM_STATE_RUN || callback == NULL || !platform_stream_start(bits)) return false;
    stream_bits = bits;
    frames = samples = number = 0;
    state = STREAM_STATE_RUN;
    return true;
}
void stream_task_stop(void) { platform_stream_stop(); state = STREAM_STATE_IDLE; }
void stream_task_handle(void)
{
    if (state != STREAM_STATE_RUN) return;
    uint32_t count;
    const uint8_t *data = platform_stream_next(&count);
    if (data == NULL) return;
    stream_header_t header = { STREAM_SYNC_WORD, number, (uint16_t)count };
    callback(&header, data, count * (stream_bits == 16 ? 2u : 1u));
    platform_stream_release();
    number++;
    frames++;
    samples += count;
}
stream_state_t stream_task_get_state(void) { return state; }
uint8_t stream_task_get_bits(void) { return stream_bits; }
uint32_t stream_task_get_frames(void) { return frames; }
uint32_t stream_task_get_samples(void) { return samples; }
uint32_t stream_task_get_overruns(void) { return platform_stream_overruns(); }
