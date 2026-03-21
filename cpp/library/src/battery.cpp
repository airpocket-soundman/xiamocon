#include "xmc/battery.h"
#include "xmc/hw/i2c.h"
#include "xmc/hw/timer.h"

static const uint8_t ADC101C021_I2C_ADDR = 0x52;

typedef enum {
  ADC101C021_ADDR_CONV_RESULT = 0x00,
  ADC101C021_ADDR_ALERT_STATUS = 0x01,
  ADC101C021_ADDR_CONFIG = 0x02,
  ADC101C021_ADDR_LOW_LIMIT = 0x03,
  ADC101C021_ADDR_HIGH_LIMIT = 0x04,
  ADC101C021_ADDR_HYSTERESIS = 0x05,
  ADC101C021_ADDR_LOWEST_CONV = 0x06,
  ADC101C021_ADDR_HIGHEST_CONV = 0x07,
} adc101c021_reg_t;

static uint64_t nextUpdateMs = 0;
static uint16_t lastMv = 0;

static XmcStatus writeRegU8(adc101c021_reg_t reg, uint8_t value);
static bool tryReadRegU16(adc101c021_reg_t reg, uint16_t *value);

XmcStatus xmc_batteryInit() {
  uint8_t config = 0;
  config |= (0 << 5);
  XMC_ERR_RET(writeRegU8(ADC101C021_ADDR_CONFIG, config));
  xmc_batteryService();
  return XMC_OK;
}

XmcStatus xmc_batteryDeinit() {
  uint8_t config = 0;
  config |= (0 << 5);
  XMC_ERR_RET(writeRegU8(ADC101C021_ADDR_CONFIG, config));
  return XMC_OK;
}

XmcStatus xmc_batteryService() {
  uint64_t now_ms = xmc_getTimeMs();
  if (now_ms < nextUpdateMs) {
    return XMC_OK;
  }
  nextUpdateMs = now_ms + 1000;
  uint16_t raw;
  if (tryReadRegU16(ADC101C021_ADDR_CONV_RESULT, &raw)) {
    lastMv = (uint32_t)((raw >> 2) & 0x3FF) * 2 * 3300 / 1024;
  }
  return XMC_OK;
}

uint16_t xmc_batteryGetVoltageMv() { return lastMv; }

static XmcStatus writeRegU8(adc101c021_reg_t reg, uint8_t value) {
  XmcStatus status = XMC_OK;
  uint8_t buf[2] = {(uint8_t)reg, value};
  XMC_ERR_RET(xmc_i2c_lock());
  do {
    XMC_ERR_BRK(status, xmc_i2cSetBaudrate(xmc_i2cGetPreferredFrequency(
                            XMC_I2C_DEV_BAT_MON)));
    XMC_ERR_BRK(status, xmc_i2cWriteBlocking(ADC101C021_I2C_ADDR, buf,
                                               sizeof(buf), false));
  } while (0);
  XMC_ERR_RET(xmc_i2cUnlock());
  return XMC_OK;
}

static bool tryReadRegU16(adc101c021_reg_t reg, uint16_t *value) {
  XmcStatus status = XMC_OK;
  uint8_t reg_addr = (uint8_t)reg;
  uint8_t data[2];
  if (!xmc_i2cTryLock()) {
    return false;
  }
  do {
    XMC_ERR_BRK(status, xmc_i2cSetBaudrate(xmc_i2cGetPreferredFrequency(
                            XMC_I2C_DEV_BAT_MON)));
    XMC_ERR_BRK(status, xmc_i2cWriteBlocking(ADC101C021_I2C_ADDR, &reg_addr,
                                               1, false));
    XMC_ERR_BRK(status,
                xmc_i2cReadBlocking(ADC101C021_I2C_ADDR, data, 2, false));
    *value = ((uint16_t)data[0] << 8) | data[1];
  } while (0);
  xmc_i2cUnlock();
  return status == XMC_OK;
}
