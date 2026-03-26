#include "xmc/hw/gpio.hpp"

#include <Arduino.h>

namespace xmc::gpio {

void setDir(int pin, bool output) {
  if (pin < 0) return;
  pinMode(pin, output ? OUTPUT : INPUT);
}

void write(int pin, bool value) {
  if (pin < 0) return;
  digitalWrite(pin, value ? HIGH : LOW);
}

bool read(int pin) {
  if (pin < 0) return false;
  return digitalRead(pin) != LOW;
}

void setPullup(int pin, bool enable) {
  if (pin < 0) return;
  pinMode(pin, enable ? INPUT_PULLUP : INPUT);
}

}  // namespace xmc::gpio
