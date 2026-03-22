#include "xmc/system.hpp"
#include "xmc/battery.hpp"
#include "xmc/display.hpp"
#include "xmc/hw/gpio.hpp"
#include "xmc/hw/i2c.hpp"
#include "xmc/hw/pins.hpp"
#include "xmc/hw/power.hpp"
#include "xmc/hw/spi.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/input.hpp"
#include "xmc/ioex.hpp"

namespace xmc::system {

XmcStatus init() {
  gpio::setDir(XMC_PIN_POWER_BUTTON, false);

  i2c::init();
  spi::init();

  ioex::init();
  ioex::setDirMasked(0, 0xFF, 0xFF);
  ioex::setDirMasked(1, 0xFF, 0xFF);

  battery::init();

  // Mute speaker during initialization to avoid noise
  ioex::write(ioex::Pin::PERI_EN, true);
  ioex::setDir(ioex::Pin::SPEAKER_MUTE, true);

  // Reset LCD
  ioex::write(ioex::Pin::DISPLAY_RESET, false);
  ioex::setDir(ioex::Pin::DISPLAY_RESET, true);

  // Power on peripherals
  ioex::write(ioex::Pin::PERI_EN, true);
  ioex::setDir(ioex::Pin::PERI_EN, true);
  xmc::sleepMs(100);
  ioex::write(ioex::Pin::PERI_EN, false);
  xmc::sleepMs(100);

  return XMC_OK;
}

XmcStatus Service() {
  battery::service();
  input::service();
  return XMC_OK;
}

XmcStatus requestShutdown() {
  display::deinit();
  input::deinit();
  battery::deinit();
  ioex::setDir(ioex::Pin::DISPLAY_RESET, false);
  ioex::setDir(ioex::Pin::SPEAKER_MUTE, false);
  ioex::write(ioex::Pin::PERI_EN, true);
  ioex::deinit();
  i2c::deinit();
  spi::deinit();
  XMC_ERR_RET(power::deepSleep());
  XMC_ERR_RET(power::reset(power::ResetMode::NORMAL));
  return XMC_OK;
}

}  // namespace xmc::system
