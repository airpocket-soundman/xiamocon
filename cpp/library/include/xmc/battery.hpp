/**
 * @file battery.hpp
 * @brief Battery monitor interface for XMC library
 */

#ifndef XMC_BATTERY_HPP
#define XMC_BATTERY_HPP

#include "xmc/xmc_common.hpp"

namespace xmc::battery {

XmcStatus init();
XmcStatus deinit();
XmcStatus service();
uint16_t getVoltageMv();

}  // namespace xmc::battery

#endif 

