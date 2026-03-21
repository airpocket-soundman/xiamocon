#include "xmc/hw/i2c.h"
#include "xmc/hw/lock.h"
#include "xmc/hw/pins.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico/stdlib.h>

static i2c_inst_t *const i2cInst = i2c1;
static Semaphore semaphore;

uint32_t xmc_i2cGetPreferredFrequency(xmc_i2c_device_t device) {
  switch (device) {
    case XMC_I2C_DEV_IOEX: return 400000;
    case XMC_I2C_DEV_BAT_MON: return 400000;
    default: return 400000;
  }
}

XmcStatus xmc_i2cInit() {
  i2c_init(i2cInst, xmc_i2cGetPreferredFrequency(XMC_I2C_DEV_IOEX));
  gpio_set_function(XMC_PIN_I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(XMC_PIN_I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(XMC_PIN_I2C_SDA);
  gpio_pull_up(XMC_PIN_I2C_SCL);
  XMC_ERR_RET(xmc_semaphoreInit(&semaphore));
  return XMC_OK;
}

void xmc_i2cDeinit() {
  i2c_deinit(i2cInst);
  gpio_deinit(XMC_PIN_I2C_SDA);
  gpio_deinit(XMC_PIN_I2C_SCL);
  xmc_semaphoreDeinit(&semaphore);
}

bool xmc_i2cTryLock() { return xmc_semaphoreTryTake(&semaphore); }

XmcStatus xmc_i2cUnlock() {
  xmc_semaphoreGive(&semaphore);
  return XMC_OK;
}

XmcStatus xmc_i2cSetBaudrate(uint32_t baudrate) {
  i2c_set_baudrate(i2cInst, baudrate);
  return XMC_OK;
}

XmcStatus xmc_i2cWriteBlocking(uint8_t dev_addr, const uint8_t *data,
                                    uint32_t size, bool nostop) {
  int n = i2c_write_blocking(i2cInst, dev_addr, data, size, nostop);
  if (n != (int)size) {
    XMC_ERR_RET(XMC_ERR_I2C_WRITE_FAILED);
  }
  return XMC_OK;
}

XmcStatus xmc_i2cReadBlocking(uint8_t dev_addr, uint8_t *data,
                                   uint32_t size, bool nostop) {
  int n = i2c_read_blocking(i2cInst, dev_addr, data, size, nostop);
  if (n != (int)size) {
    XMC_ERR_RET(XMC_ERR_I2C_READ_FAILED);
  }
  return XMC_OK;
}
