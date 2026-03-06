#ifndef XMC_HW_GPIO_H
#define XMC_HW_GPIO_H

#include "xmc/hw/hw_common.h"

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

xmc_status_t xmc_gpio_set_dir(int pin, bool output);
void xmc_gpio_write(int pin, bool value);
bool xmc_gpio_read(int pin);
void xmc_gpio_set_pullup(int pin, bool enable);

#if defined(__cplusplus)
}
#endif

#endif
