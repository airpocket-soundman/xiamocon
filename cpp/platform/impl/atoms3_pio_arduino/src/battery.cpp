#include "xmc/battery.hpp"

// Stub: AtomS3 has no battery monitor IC.

namespace xmc::battery {

XmcStatus init() { return XMC_OK; }
XmcStatus deinit() { return XMC_OK; }
XmcStatus service() { return XMC_OK; }
uint16_t getVoltageMv() { return 0; }

}  // namespace xmc::battery
