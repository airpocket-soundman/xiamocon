#include "xmc/system.h"
#include "xmc/display.h"
#include "xmc/hw/gpio.h"
#include "xmc/hw/i2c.h"
#include "xmc/hw/pins.h"
#include "xmc/hw/power.h"
#include "xmc/hw/spi.h"
#include "xmc/hw/timer.h"
#include "xmc/input.h"
#include "xmc/ioex.h"
#include "xmc/battery.h"

XmcStatus xmc_sysInit() {
  xmc_gpioSetDir(XMC_PIN_POWER_BUTTON, false);

  xmc_i2cInit();
  xmc_spiInit();
  
  xmc_ioexInit();
  xmc_ioexSetDirMasked(0, 0xFF, 0xFF);
  xmc_ioexSetDirMasked(1, 0xFF, 0xFF);

  xmc_batteryInit();

  // Mute speaker during initialization to avoid noise
  xmc_ioexWrite(XMC_IOEX_PIN_PERI_EN, true);
  xmc_ioexSetDir(XMC_IOEX_PIN_SPEAKER_MUTE, true);

  // Reset LCD
  xmc_ioexWrite(XMC_IOEX_PIN_DISPLAY_RESET, false);
  xmc_ioexSetDir(XMC_IOEX_PIN_DISPLAY_RESET, true);

  // Power on peripherals
  xmc_ioexWrite(XMC_IOEX_PIN_PERI_EN, true);
  xmc_ioexSetDir(XMC_IOEX_PIN_PERI_EN, true);
  xmc_sleepMs(100);
  xmc_ioexWrite(XMC_IOEX_PIN_PERI_EN, false);
  xmc_sleepMs(100);

  return XMC_OK;
}

XmcStatus xmc_sysService() {
  xmc_batteryService();
  xmc_inputService();
  return XMC_OK;
}

XmcStatus xmc_sysRequestShutdown() {
  xmc_displayDeinit();
  xmc_inputDeinit();
  xmc_batteryDeinit();
  xmc_ioexSetDir(XMC_IOEX_PIN_DISPLAY_RESET, false);
  xmc_ioexSetDir(XMC_IOEX_PIN_SPEAKER_MUTE, false);
  xmc_ioexWrite(XMC_IOEX_PIN_PERI_EN, true);
  xmc_ioexDeinit();
  xmc_i2cDeinit();
  xmc_spiDeinit();
  XMC_ERR_RET(xmc_powerDeepSleep());
  XMC_ERR_RET(xmc_powerReset(XMC_RESET_MODE_NORMAL));
  return XMC_OK;
}
