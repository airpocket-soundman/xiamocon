#include "xmc/hw/pwm.hpp"

// Stub implementation: PWM is not used in the AtomS3 demo.

namespace xmc::pwm {

Driver::Driver(int pin) : pin(pin) { handle = (void *)1; }

Driver::~Driver() { handle = nullptr; }

XmcStatus Driver::start(const Config &cfg, float *actualFreqHz) {
  if (actualFreqHz) *actualFreqHz = (float)cfg.freqHz;
  return XMC_OK;
}

XmcStatus Driver::stop() { return XMC_OK; }

XmcStatus Driver::setDutyCycle(uint32_t) { return XMC_OK; }

}  // namespace xmc::pwm
