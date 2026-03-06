#ifndef XMC_HW_SPI_H
#define XMC_HW_SPI_H

#include "xmc/hw/hw_common.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  XMC_SPI_DEV_DISPLAY = (1 << 1),
  XMC_SPI_DEV_TFCARD = (1 << 2),
} xmc_spi_device_t;

#if defined(__cplusplus)
extern "C" {
#endif

uint32_t xmc_spi_get_preferred_frequency(xmc_spi_device_t device);
xmc_status_t xmc_spi_init();
void xmc_spi_deinit();
xmc_status_t xmc_spi_begin_transaction();
xmc_status_t xmc_spi_end_transaction();
xmc_status_t xmc_spi_set_baudrate(uint32_t baudrate);
xmc_status_t xmc_spi_write_blocking(const uint8_t *data, uint32_t size);
xmc_status_t xmc_spi_read_blocking(uint8_t repeated_byte, uint8_t *data,
                                   uint32_t size);
xmc_status_t xmc_spi_dma_write_start(const xmc_dma_config_t *cfg, int cs_pin);
xmc_status_t xmc_spi_dma_complete();
bool xmc_spi_dma_is_busy();

#if defined(__cplusplus)
}
#endif

#endif
