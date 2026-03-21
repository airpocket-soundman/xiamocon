#include "xmc/ioex.h"
#include "xmc/hw/i2c.h"

typedef enum {
  REG_GPIOA = 0x00,
  REG_GPIOB = 0x01,
  REG_OLATA = 0x02,
  REG_OLATB = 0x03,
  REG_IPOLA = 0x04,
  REG_IPOLB = 0x05,
  REG_IODIRA = 0x06,
  REG_IODIRB = 0x07,
} reg_t;

static const uint8_t DEV_ADDR = 0x20;

static uint8_t dir[2] = {0xFF, 0x00};
static uint8_t out[2] = {0xFF, 0xFF};

static XmcStatus write_reg(reg_t reg, uint8_t value);
static XmcStatus read_reg(reg_t reg, uint8_t *value);

XmcStatus xmc_ioexInit() {
  XmcStatus status = XMC_OK;
  XmcStatus s;
  s = xmc_ioexSetDirMasked(0, 0xFF, 0xFF);
  if (s != XMC_OK) {
    XMC_ERR_LOG(s);
    status = s;
  }
  s = xmc_ioexSetDirMasked(1, 0xFF, 0x00);
  if (s != XMC_OK) {
    XMC_ERR_LOG(s);
    status = s;
  }
  return status;
}

XmcStatus xmc_ioexDeinit() {
  XmcStatus status = XMC_OK;
  XmcStatus s;
  s = xmc_ioexSetDirMasked(0, 0xFF, 0x00);
  if (s != XMC_OK) {
    XMC_ERR_LOG(s);
    status = s;
  }
  s = xmc_ioexSetDirMasked(1, 0xFF, 0x00);
  if (s != XMC_OK) {
    XMC_ERR_LOG(s);
    status = s;
  }
  return status;
}

XmcStatus xmc_ioexSetDirMasked(int port, uint8_t mask, uint8_t value) {
  dir[port] = (dir[port] & ~mask) | (value & mask);
  return write_reg(port == 0 ? REG_IODIRA : REG_IODIRB, ~dir[port]);
}

XmcStatus xmc_ioexWriteMasked(int port, uint8_t mask, uint8_t value) {
  out[port] = (out[port] & ~mask) | (value & mask);
  return write_reg(port == 0 ? REG_OLATA : REG_OLATB, out[port]);
}

XmcStatus xmc_ioexReadMasked(int port, uint8_t mask, uint8_t *value) {
  uint8_t reg_val;
  XMC_ERR_RET(read_reg(port == 0 ? REG_GPIOA : REG_GPIOB, &reg_val));
  *value = reg_val & mask;
  return XMC_OK;
}

static XmcStatus write_reg(reg_t reg, uint8_t value) {
  uint8_t buf[2] = {(uint8_t)reg, value};
  XMC_ERR_RET(xmc_i2c_lock());
  XMC_ERR_RET(
      xmc_i2cSetBaudrate(xmc_i2cGetPreferredFrequency(XMC_I2C_DEV_IOEX)));
  XMC_ERR_RET(xmc_i2cWriteBlocking(DEV_ADDR, buf, 2, false));
  XMC_ERR_RET(xmc_i2cUnlock());
  return XMC_OK;
}

static XmcStatus read_reg(reg_t reg, uint8_t *value) {
  uint8_t reg_addr = (uint8_t)reg;
  XMC_ERR_RET(xmc_i2c_lock());
  XMC_ERR_RET(
      xmc_i2cSetBaudrate(xmc_i2cGetPreferredFrequency(XMC_I2C_DEV_IOEX)));
  XMC_ERR_RET(xmc_i2cWriteBlocking(DEV_ADDR, &reg_addr, 1, false));
  XMC_ERR_RET(xmc_i2cReadBlocking(DEV_ADDR, value, 1, false));
  XMC_ERR_RET(xmc_i2cUnlock());
  return XMC_OK;
}

XmcStatus xmc_ioexReadAll(uint16_t *value) {
  uint8_t reg_addr = (uint8_t)REG_GPIOA;
  uint8_t data[2];
  XMC_ERR_RET(xmc_i2c_lock());
  XMC_ERR_RET(
      xmc_i2cSetBaudrate(xmc_i2cGetPreferredFrequency(XMC_I2C_DEV_IOEX)));
  XMC_ERR_RET(xmc_i2cWriteBlocking(DEV_ADDR, &reg_addr, 1, false));
  XMC_ERR_RET(xmc_i2cReadBlocking(DEV_ADDR, data, 2, false));
  XMC_ERR_RET(xmc_i2cUnlock());
  *value = data[0] | (((uint16_t)data[1]) << 8);
  return XMC_OK;
}

bool xmc_ioexTryReadAll(uint16_t *value) {
  XmcStatus status;
  uint8_t reg_addr = (uint8_t)REG_GPIOA;
  uint8_t data[2];
  if (!xmc_i2cTryLock()) {
    return false;
  }
  do {
    XMC_ERR_BRK(status, xmc_i2cSetBaudrate(
                            xmc_i2cGetPreferredFrequency(XMC_I2C_DEV_IOEX)));
    XMC_ERR_BRK(status, xmc_i2cWriteBlocking(DEV_ADDR, &reg_addr, 1, false));
    XMC_ERR_BRK(status, xmc_i2cReadBlocking(DEV_ADDR, data, 2, false));
    *value = data[0] | (((uint16_t)data[1]) << 8);
  } while (0);
  xmc_i2cUnlock();
  return status == XMC_OK;
}