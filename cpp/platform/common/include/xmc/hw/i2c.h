#ifndef XMC_HW_I2C_H
#define XMC_HW_I2C_H

#include "xmc/hw/hw_common.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  XMC_I2C_DEV_IOEX = (1 << 1),
  XMC_I2C_DEV_BAT_MON = (1 << 2),
} xmc_i2c_device_t;

#if defined(__cplusplus)
extern "C" {
#endif

uint32_t xmc_i2c_get_preferred_frequency(xmc_i2c_device_t device);
xmc_status_t xmc_i2c_init();
void xmc_i2c_deinit();
xmc_status_t xmc_i2c_start_transaction();
xmc_status_t xmc_i2c_end_transaction();
xmc_status_t xmc_i2c_set_baudrate(uint32_t baudrate);
xmc_status_t xmc_i2c_write_blocking(uint8_t dev_addr, const uint8_t *data,
                            uint32_t size, bool nostop);
xmc_status_t xmc_i2c_read_blocking(uint8_t dev_addr, uint8_t *data, uint32_t size,
                           bool nostop);

#if defined(__cplusplus)
}
#endif

#endif
