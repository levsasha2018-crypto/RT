#include "block-task/block-task.h"
#include "platform/platform.h"
#include <stddef.h>
static block_task_callback_t callback;
static uint32_t block_count;
bool block_task_start(uint32_t count, block_task_callback_t on_block)
{
    if (callback != NULL || on_block == NULL || !platform_block_start(count)) return false;
    block_count = count;
    callback = on_block;
    return true;
}
void block_task_handle(void)
{
    if (callback == NULL || !platform_block_done()) return;
    block_task_callback_t done = callback;
    callback = NULL;
    done(platform_block_samples(), block_count, platform_block_time_us());
}
