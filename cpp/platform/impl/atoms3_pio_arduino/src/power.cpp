#include "xmc/hw/power.hpp"

#include <Arduino.h>
#include <esp_system.h>

namespace xmc::power {

XmcStatus init() { return XMC_OK; }

XmcStatus service() { return XMC_OK; }

XmcStatus deepSleep() {
  esp_deep_sleep_start();
  return XMC_OK;
}

XmcStatus reset(ResetMode) {
  esp_restart();
  return XMC_ERR_POWER_RESET_FAILED;
}

}  // namespace xmc::power
