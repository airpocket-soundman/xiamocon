#ifndef XMC_HW_POWER_H
#define XMC_HW_POWER_H

#include "xmc/hw/hw_common.h"

#include <stdint.h>

typedef enum {
    XMC_RESET_MODE_NORMAL = 0,
} xmc_reset_mode_t;

xmc_status_t xmc_power_init();
xmc_status_t xmc_power_service();
xmc_status_t xmc_power_deep_sleep();
xmc_status_t xmc_power_reset(xmc_reset_mode_t mode);

#endif
