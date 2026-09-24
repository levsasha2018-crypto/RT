#pragma once

#include <stdint.h>

#define DEVICE_NAME "es-cmd-usb"
#define FIRMWARE_VERSION "1.0.0"
#define DEVICE_PROJECT "211-command-usb"
#ifndef DEVICE_REPO
#define DEVICE_REPO "https://github.com/levsasha2018-crypto/RT"
#endif
#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

struct info_t
{
    uint32_t version;
    char name[13];
    uint8_t revision;
};

extern struct info_t device_card;

void device_info(void);
void dev_info(void);
