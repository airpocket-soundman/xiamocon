#ifndef XMC_SYSTEM_H
#define XMC_SYSTEM_H

#include "xmc/xmc_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

xmc_status_t xmc_sys_init();
xmc_status_t xmc_sys_shutdown();

#if defined(__cplusplus)
}
#endif

#endif // XMC_SYSTEM_H
