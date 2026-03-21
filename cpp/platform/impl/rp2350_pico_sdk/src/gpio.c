#include "xmc/hw/gpio.h"
#include "xmc/hw/hw_common.h"

#include <hardware/gpio.h>

#if defined(__cplusplus)
extern "C" {
#endif

void xmc_gpioSetDir(int pin, bool output) {
  gpio_init(pin);
  gpio_set_dir(pin, output ? GPIO_OUT : GPIO_IN);
}

void xmc_gpioWrite(int pin, bool value) { gpio_put(pin, value ? 1 : 0); }

bool xmc_gpioRead(int pin) { return gpio_get(pin) != 0; }

void xmc_gpioSetPullup(int pin, bool enable) {
  gpio_set_pulls(pin, enable, false);
}

#if defined(__cplusplus)
}
#endif
