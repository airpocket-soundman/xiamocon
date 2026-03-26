#ifndef MPU6886_HPP
#define MPU6886_HPP

#include <xmc/hw/i2c.hpp>

// MPU6886 6-axis IMU driver (I2C).
// Built into M5Stack AtomS3 at address 0x68.
//
// read_sensor() returns 7 floats in the same order as the lsm6dsv16x driver:
//   values[0] = temperature (°C)
//   values[1..3] = gyroscope  (dps, X/Y/Z)
//   values[4..6] = accelerometer (G, X/Y/Z)

namespace mpu6886 {

using namespace xmc;

enum reg_t : uint8_t {
  REG_SMPLRT_DIV  = 0x19,
  REG_CONFIG      = 0x1A,
  REG_GYRO_CONFIG = 0x1B,
  REG_ACCEL_CONFIG = 0x1C,
  REG_INT_ENABLE  = 0x38,
  REG_ACCEL_XOUT_H = 0x3B,
  REG_TEMP_OUT_H  = 0x41,
  REG_GYRO_XOUT_H = 0x43,
  REG_PWR_MGMT_1  = 0x6B,
  REG_WHO_AM_I    = 0x75,
};

static constexpr uint8_t WHO_AM_I_VAL = 0x19;

class SensorI2C {
 public:
  const uint8_t dev_addr;
  const uint32_t baudrate_hz;

 private:
  bool inited = false;

 public:
  SensorI2C(uint8_t addr = 0x68, uint32_t baud = 400000)
      : dev_addr(addr), baudrate_hz(baud) {}

  XmcStatus init() {
    inited = false;

    // Wake up (clear sleep bit)
    XMC_ERR_RET(write_reg(REG_PWR_MGMT_1, 0x00));
    xmc::sleepMs(10);

    // Software reset
    XMC_ERR_RET(write_reg(REG_PWR_MGMT_1, 0x80));
    xmc::sleepMs(10);

    // Clock source: auto-select best available
    XMC_ERR_RET(write_reg(REG_PWR_MGMT_1, 0x01));
    xmc::sleepMs(10);

    // Verify WHO_AM_I
    uint8_t who_am_i = 0;
    XMC_ERR_RET(read_reg(REG_WHO_AM_I, &who_am_i));
    if (who_am_i != WHO_AM_I_VAL) {
      return XMC_USER_GENERIC_ERROR;
    }

    // Disable interrupts
    XMC_ERR_RET(write_reg(REG_INT_ENABLE, 0x00));

    // Sample rate divider: 0 → 1 kHz (when DLPF enabled)
    XMC_ERR_RET(write_reg(REG_SMPLRT_DIV, 0x00));

    // DLPF: bandwidth 188 Hz
    XMC_ERR_RET(write_reg(REG_CONFIG, 0x01));

    // Gyro full-scale: ±250 dps (FS_SEL = 0), sensitivity = 131 LSB/dps
    XMC_ERR_RET(write_reg(REG_GYRO_CONFIG, 0x00));

    // Accel full-scale: ±2 G (AFS_SEL = 0), sensitivity = 16384 LSB/G
    XMC_ERR_RET(write_reg(REG_ACCEL_CONFIG, 0x00));

    xmc::sleepMs(10);
    inited = true;
    return XMC_OK;
  }

  // values: [temp_C, gx_dps, gy_dps, gz_dps, ax_G, ay_G, az_G]
  XmcStatus read_sensor(float *values) {
    int16_t raw[7] = {0};
    XMC_ERR_RET(read_sensor_raw(raw));
    // MPU6886 temperature formula
    values[0] = raw[0] / 326.8f + 25.0f;
    // Gyro: ±250 dps → 131 LSB/dps
    values[1] = raw[1] / 131.0f;
    values[2] = raw[2] / 131.0f;
    values[3] = raw[3] / 131.0f;
    // Accel: ±2 G → 16384 LSB/G
    values[4] = raw[4] / 16384.0f;
    values[5] = raw[5] / 16384.0f;
    values[6] = raw[6] / 16384.0f;
    return XMC_OK;
  }

  // Raw sensor values in the order: [temp, gx, gy, gz, ax, ay, az]
  XmcStatus read_sensor_raw(int16_t *values) {
    if (!inited) return XMC_USER_GENERIC_ERROR;
    // 14 bytes: ACCEL_XOUT_H … GYRO_ZOUT_L  (accel, temp, gyro — contiguous)
    uint8_t buf[14] = {0};
    XMC_ERR_RET(read_reg(REG_ACCEL_XOUT_H, buf, sizeof(buf)));
    int16_t ax = (int16_t)((buf[0]  << 8) | buf[1]);
    int16_t ay = (int16_t)((buf[2]  << 8) | buf[3]);
    int16_t az = (int16_t)((buf[4]  << 8) | buf[5]);
    int16_t t  = (int16_t)((buf[6]  << 8) | buf[7]);
    int16_t gx = (int16_t)((buf[8]  << 8) | buf[9]);
    int16_t gy = (int16_t)((buf[10] << 8) | buf[11]);
    int16_t gz = (int16_t)((buf[12] << 8) | buf[13]);
    // Re-order to match lsm6dsv16x convention: [temp, gx, gy, gz, ax, ay, az]
    values[0] = t;
    values[1] = gx;
    values[2] = gy;
    values[3] = gz;
    values[4] = ax;
    values[5] = ay;
    values[6] = az;
    return XMC_OK;
  }

  XmcStatus read_reg(reg_t reg, uint8_t *value, uint32_t num_bytes = 1) {
    XmcStatus sts = XMC_OK;
    XMC_ERR_RET(i2c::lock());
    do {
      XMC_ERR_BRK(sts, i2c::setBaudrate(baudrate_hz));
      XMC_ERR_BRK(sts, i2c::writeBlocking(dev_addr, (uint8_t *)&reg, 1, false));
      XMC_ERR_BRK(sts, i2c::readBlocking(dev_addr, value, num_bytes, false));
    } while (0);
    XMC_ERR_RET(i2c::unlock());
    return sts;
  }

  XmcStatus write_reg(reg_t reg, uint8_t data) {
    XmcStatus sts = XMC_OK;
    XMC_ERR_RET(i2c::lock());
    do {
      XMC_ERR_BRK(sts, i2c::setBaudrate(baudrate_hz));
      uint8_t buf[2] = {(uint8_t)reg, data};
      XMC_ERR_BRK(sts, i2c::writeBlocking(dev_addr, buf, 2, false));
    } while (0);
    XMC_ERR_RET(i2c::unlock());
    return sts;
  }
};

}  // namespace mpu6886

#endif  // MPU6886_HPP
