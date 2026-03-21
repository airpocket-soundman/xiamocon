/**
 * @file i2c.h
 * @brief I2C hardware interface
 */

#ifndef XMC_HW_I2C_H
#define XMC_HW_I2C_H

#include "xmc/hw/hw_common.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * I2C device identifiers for getting preferred baud rates.
 */
typedef enum {
  /** IO Expander */
  XMC_I2C_DEV_IOEX = (1 << 1),
  /** Battery Monitor */
  XMC_I2C_DEV_BAT_MON = (1 << 2),
} xmc_i2c_device_t;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Get the preferred I2C frequency for a given device. This can be used to set
 * the baud rate when initializing the I2C peripheral.
 * @param device The I2C device identifier.
 * @return The preferred I2C frequency in Hz.
 */
uint32_t xmc_i2cGetPreferredFrequency(xmc_i2c_device_t device);

/**
 * Initialize the I2C peripheral and configure the GPIO pins for I2C
 * functionality. This must be called before any other I2C functions except
 * xmc_i2cGetPreferredFrequency.
 * @return XMC_OK if the I2C peripheral was successfully initialized.
 */
XmcStatus xmc_i2cInit();

/** Deinitialize the I2C peripheral and release any resources. */
void xmc_i2cDeinit();

/**
 * Try to start an I2C transaction without blocking. This can be used in
 * situations where blocking is not acceptable, such as in an interrupt handler
 * or a critical section. If the transaction cannot be started immediately
 * because another transaction is in progress, this function will return false.
 * Otherwise, it will return true and the caller can proceed with I2C
 * operations.
 * @return true if the transaction was successfully started, false otherwise.
 */
bool xmc_i2cTryLock();

/**
 * Start an I2C transaction.
 *
 * This function must be called before accessing I2C devices or changing I2C
 * peripheral settings. I2C transactions from other tasks will be blocked until
 * xmc_i2cUnlock is called.
 *
 * @return XMC_OK if the transaction was successfully started.
 */
static inline XmcStatus xmc_i2c_lock() {
  while (!xmc_i2cTryLock()) {
    xmc_tightLoopContents();
  }
  return XMC_OK;
}

/**
 * End an I2C transaction.
 *
 * This function must be called after accessing I2C devices or changing I2C
 * peripheral settings.
 *
 * @return XMC_OK if the transaction was successfully ended.
 */
XmcStatus xmc_i2cUnlock();

/**
 * Set the I2C baud rate.
 * @param baudrate The desired I2C baud rate in Hz. The actual baud rate may be
 * adjusted to the nearest supported value.
 */
XmcStatus xmc_i2cSetBaudrate(uint32_t baudrate);

/**
 * Write data to an I2C device in a blocking manner.
 * @param dev_addr The 7-bit I2C address of the target device.
 * @param data Pointer to the data buffer to be sent.
 * @param size The number of bytes to write from the data buffer.
 * @param nostop If true, do not send a STOP condition after the write.
 */
XmcStatus xmc_i2cWriteBlocking(uint8_t dev_addr, const uint8_t *data,
                                    uint32_t size, bool nostop);

/**
 * Read data from an I2C device in a blocking manner.
 * @param dev_addr The 7-bit I2C address of the target device.
 * @param data Pointer to the data buffer to store the received data.
 * @param size The number of bytes to read into the data buffer.
 * @param nostop If true, do not send a STOP condition after the read.
 */
XmcStatus xmc_i2cReadBlocking(uint8_t dev_addr, uint8_t *data,
                                   uint32_t size, bool nostop);

#if defined(__cplusplus)
}
#endif

#endif
