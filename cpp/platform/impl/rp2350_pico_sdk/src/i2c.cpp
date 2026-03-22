#include "xmc/hw/i2c.hpp"
#include "xmc/hw/pins.hpp"
#include "xmc/hw/semaphore.hpp"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico/stdlib.h>

namespace xmc::i2c {

static i2c_inst_t *const i2cInst = i2c1;
static Semaphore *semaphore = nullptr;

uint32_t getPreferredFrequency(Chipset device) {
  switch (device) {
    case Chipset::IO_EXPANDER: return 400000;
    case Chipset::BATTERY_MONITOR: return 400000;
    default: return 400000;
  }
}

XmcStatus init() {
  i2c_init(i2cInst, getPreferredFrequency(Chipset::IO_EXPANDER));
  gpio_set_function(XMC_PIN_I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(XMC_PIN_I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(XMC_PIN_I2C_SDA);
  gpio_pull_up(XMC_PIN_I2C_SCL);
  if (semaphore) delete semaphore;
  semaphore = new Semaphore();
  if (!semaphore->isInitialized()) {
    XMC_ERR_RET(XMC_ERR_SEMAPHORE_INIT_FAILED);
  }
  return XMC_OK;
}

void deinit() {
  i2c_deinit(i2cInst);
  gpio_deinit(XMC_PIN_I2C_SDA);
  gpio_deinit(XMC_PIN_I2C_SCL);
  if (semaphore) {
    delete semaphore;
    semaphore = nullptr;
  }
}

bool tryLock() {
  if (!semaphore) return true;
  return semaphore->tryTake();
}

XmcStatus unlock() {
  if (semaphore) semaphore->give();
  return XMC_OK;
}

XmcStatus setBaudrate(uint32_t baudrate) {
  i2c_set_baudrate(i2cInst, baudrate);
  return XMC_OK;
}

XmcStatus writeBlocking(uint8_t dev_addr, const uint8_t *data, uint32_t size,
                        bool nostop) {
  int n = i2c_write_blocking(i2cInst, dev_addr, data, size, nostop);
  if (n != (int)size) {
    XMC_ERR_RET(XMC_ERR_I2C_WRITE_FAILED);
  }
  return XMC_OK;
}

XmcStatus readBlocking(uint8_t dev_addr, uint8_t *data, uint32_t size,
                       bool nostop) {
  int n = i2c_read_blocking(i2cInst, dev_addr, data, size, nostop);
  if (n != (int)size) {
    XMC_ERR_RET(XMC_ERR_I2C_READ_FAILED);
  }
  return XMC_OK;
}

}  // namespace xmc::i2c
