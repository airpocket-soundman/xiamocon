#ifndef XMC_BATTERY_H
#define XMC_BATTERY_H

#include "xmc/xmc_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

XmcStatus xmc_batteryInit();
XmcStatus xmc_batteryDeinit();
XmcStatus xmc_batteryService();
uint16_t xmc_batteryGetVoltageMv();

#if defined(__cplusplus)
}
#endif

#endif  // XMC_BATTERY_H

