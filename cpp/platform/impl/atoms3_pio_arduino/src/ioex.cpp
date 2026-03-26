#include "xmc/ioex.hpp"
#include "xmc/hw/gpio.hpp"

#include <Arduino.h>

// AtomS3 IO expander substitute:
//   Pin::DISPLAY_RESET (5)  -> GPIO34 (active low)
//   Pin::BTN_A         (8)  -> GPIO41 (active low, internal pull-up)
// All other pins are no-ops (outputs ignored, inputs read as HIGH/inactive).

static const int ATOMS3_PIN_DISPLAY_RST = 34;
static const int ATOMS3_PIN_BTN_A = 41;

namespace xmc::ioex {

XmcStatus init() {
  gpio::setDir(ATOMS3_PIN_DISPLAY_RST, true);
  gpio::write(ATOMS3_PIN_DISPLAY_RST, 1);
  pinMode(ATOMS3_PIN_BTN_A, INPUT_PULLUP);
  return XMC_OK;
}

XmcStatus deinit() { return XMC_OK; }

XmcStatus setDirMasked(int port, uint8_t mask, uint8_t value) {
  // Port 0 bit 5 = DISPLAY_RESET
  if (port == 0 && (mask & (1 << 5))) {
    gpio::setDir(ATOMS3_PIN_DISPLAY_RST, (value >> 5) & 1);
  }
  return XMC_OK;
}

XmcStatus writeMasked(int port, uint8_t mask, uint8_t value) {
  // Port 0 bit 5 = DISPLAY_RESET
  if (port == 0 && (mask & (1 << 5))) {
    gpio::write(ATOMS3_PIN_DISPLAY_RST, (value >> 5) & 1);
  }
  return XMC_OK;
}

XmcStatus readMasked(int port, uint8_t mask, uint8_t *value) {
  *value = 0xFF & mask;  // default: all inactive (high)
  // Port 1 bit 0 = BTN_A (active low)
  if (port == 1 && (mask & 0x01)) {
    if (digitalRead(ATOMS3_PIN_BTN_A) == LOW) {
      *value &= ~0x01;
    }
  }
  return XMC_OK;
}

XmcStatus readAll(uint16_t *value) {
  *value = 0xFFFF;
  if (digitalRead(ATOMS3_PIN_BTN_A) == LOW) {
    *value &= ~(uint16_t)(1u << (int)Pin::BTN_A);
  }
  return XMC_OK;
}

bool tryReadAll(uint16_t *value) {
  return readAll(value) == XMC_OK;
}

}  // namespace xmc::ioex
