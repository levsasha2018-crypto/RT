#pragma once
#define DEVICE_NAME "es-led-module"
#define FIRMWARE_VERSION "1.0.0"
#define DEVICE_PROJECT "134-led-module"
#ifndef DEVICE_REPO
#define DEVICE_REPO "https://github.com/levsasha2018-crypto/RT"
#endif
#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif
void device_info(void);
