#include "xmc/hw/spi.hpp"
#include "xmc/hw/gpio.hpp"
#include "xmc/hw/pins.hpp"

#include <Arduino.h>
#include <SPI.h>

namespace xmc::spi {

static bool s_locked = false;
static uint32_t s_baudrate = 10000000;
static int s_pending_cs = -1;

uint32_t getPreferredFrequency(Chipset device) {
  switch (device) {
    case Chipset::DISPLAY: return 40000000;
    default: return 10000000;
  }
}

XmcStatus init() {
  SPI.begin(XMC_PIN_SPI_SCK, XMC_PIN_SPI_MISO, XMC_PIN_SPI_MOSI, -1);
  return XMC_OK;
}

void deinit() { SPI.end(); }

bool tryLock() {
  if (s_locked) return false;
  s_locked = true;
  SPI.beginTransaction(SPISettings(s_baudrate, MSBFIRST, SPI_MODE0));
  return true;
}

XmcStatus unlock() {
  dmaComplete();
  SPI.endTransaction();
  s_locked = false;
  return XMC_OK;
}

XmcStatus setBaudrate(uint32_t baudrate) {
  if (baudrate == s_baudrate) return XMC_OK;
  s_baudrate = baudrate;
  SPI.endTransaction();
  SPI.beginTransaction(SPISettings(s_baudrate, MSBFIRST, SPI_MODE0));
  return XMC_OK;
}

XmcStatus writeBlocking(const uint8_t *data, uint32_t size) {
  SPI.writeBytes(data, size);
  return XMC_OK;
}

XmcStatus readBlocking(uint8_t repeated_byte, uint8_t *data, uint32_t size) {
  memset(data, repeated_byte, size);
  SPI.transferBytes(data, data, size);
  return XMC_OK;
}

XmcStatus dmaWriteStart(const dma::Config *cfg, int csPin) {
  if (csPin >= 0) gpio::write(csPin, 0);
  s_pending_cs = csPin;
  SPI.writeBytes((const uint8_t *)cfg->ptr,
                 (uint32_t)cfg->element_size * (uint32_t)cfg->length);
  return XMC_OK;
}

XmcStatus dmaComplete() {
  if (s_pending_cs >= 0) {
    gpio::write(s_pending_cs, 1);
    s_pending_cs = -1;
  }
  return XMC_OK;
}

bool dmaIsBusy() { return false; }

}  // namespace xmc::spi
