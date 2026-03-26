#include "xmc/app_entry.hpp"

#include <Arduino.h>

void setup() {
  xmc::appMain();
}

void loop() {
  // appMain() never returns; this is unreachable.
}
