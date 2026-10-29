#include "device/device.h"

#include <stdint.h>

#include "hardware/regs/addressmap.h"
#include "hardware/regs/sysinfo.h"
#include "pico/unique_id.h"
#include "pico/version.h"

void device_get_info(device_info_t *info)
{
    pico_get_unique_board_id_string(info->serial, sizeof(info->serial));
    const uint32_t id = *(const volatile uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    info->manufacturer =
        (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> SYSINFO_CHIP_ID_MANUFACTURER_LSB;
    info->part = (id & SYSINFO_CHIP_ID_PART_BITS) >> SYSINFO_CHIP_ID_PART_LSB;
    info->revision =
        (id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB;

    info->board = DEVICE_BOARD;
    info->sdk_version = PICO_SDK_VERSION_STRING;
}
