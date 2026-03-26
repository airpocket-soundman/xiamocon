#include "xmc/hw/i2c.hpp"
#include "xmc/hw/pins.hpp"

#include <Arduino.h>
#include <Wire.h>

namespace xmc::i2c {

static bool s_locked = false;
static uint32_t s_baudrate = 400000;

uint32_t getPreferredFrequency(Chipset) { return 400000; }

XmcStatus init() {
  Wire.begin(XMC_PIN_I2C_SDA, XMC_PIN_I2C_SCL);
  Wire.setClock(s_baudrate);
  return XMC_OK;
}

void deinit() { Wire.end(); }

bool tryLock() {
  if (s_locked) return false;
  s_locked = true;
  return true;
}

XmcStatus unlock() {
  s_locked = false;
  return XMC_OK;
}

XmcStatus setBaudrate(uint32_t baudrate) {
  if (baudrate != s_baudrate) {
    s_baudrate = baudrate;
    Wire.setClock(baudrate);
  }
  return XMC_OK;
}

XmcStatus writeBlocking(uint8_t dev_addr, const uint8_t *data, uint32_t size,
                        bool nostop) {
  Wire.beginTransmission(dev_addr);
  Wire.write(data, size);
  uint8_t result = Wire.endTransmission(!nostop);
  if (result != 0) XMC_ERR_RET(XMC_ERR_I2C_WRITE_FAILED);
  return XMC_OK;
}

XmcStatus readBlocking(uint8_t dev_addr, uint8_t *data, uint32_t size,
                       bool nostop) {
  (void)nostop;
  uint32_t n = Wire.requestFrom((uint8_t)dev_addr, (uint8_t)size);
  if (n != size) XMC_ERR_RET(XMC_ERR_I2C_READ_FAILED);
  for (uint32_t i = 0; i < size; i++) {
    data[i] = Wire.read();
  }
  return XMC_OK;
}

}  // namespace xmc::i2c
