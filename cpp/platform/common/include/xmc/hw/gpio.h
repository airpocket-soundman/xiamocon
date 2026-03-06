#ifndef XMC_HW_GPIO_H
#define XMC_HW_GPIO_H

#include "xmc/hw/hw_common.h"

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Set the direction of a GPIO pin.
 * @param pin The GPIO pin number.
 * @param output If true, set the pin as an output. If false, set it as an input.
 */
void xmc_gpio_set_dir(int pin, bool output);

/**
 * Write a value to a GPIO pin.
 * @param pin The GPIO pin number.
 * @param value The value to write. True for high, false for low.
 */
void xmc_gpio_write(int pin, bool value);

/**
 * Read the value of a GPIO pin.
 * @param pin The GPIO pin number.
 * @return True if the pin is high, false if it is low.
 */
bool xmc_gpio_read(int pin);

/**
 * Set the pull-up resistor of a GPIO pin.
 * @param pin The GPIO pin number.
 * @param enable If true, enable the pull-up resistor. If false, disable it.
 */
void xmc_gpio_set_pullup(int pin, bool enable);

#if defined(__cplusplus)
}
#endif

#endif
