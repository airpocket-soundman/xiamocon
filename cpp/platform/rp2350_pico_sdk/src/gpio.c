#include "xmc/hw/gpio.h"
#include "xmc/hw/hw_common.h"

#include <hardware/gpio.h>

#if defined(__cplusplus)
extern "C" {
#endif

xmc_status_t xmc_gpio_set_dir(int pin, bool output) {
  gpio_init(pin);
  gpio_set_dir(pin, output ? GPIO_OUT : GPIO_IN);
  return XMC_OK;
}

void xmc_gpio_write(int pin, bool value) { gpio_put(pin, value ? 1 : 0); }

bool xmc_gpio_read(int pin) { return gpio_get(pin) != 0; }

void xmc_gpio_set_pullup(int pin, bool enable) {
  gpio_set_pulls(pin, enable, false);
}

#if defined(__cplusplus)
}
#endif
