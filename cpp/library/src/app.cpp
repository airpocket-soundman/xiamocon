#include "xmc/app.h"
#include "xmc/app_entry.h"
#include "xmc/hw/timer.h"
#include "xmc/input.h"
#include "xmc/speaker.h"
#include "xmc/system.h"

#include <stddef.h>

AppConfig xmcGetDefaultAppConfig() {
  AppConfig config = {
      .displayPixelFormat = XMC_DISP_INTF_FORMAT_RGB565,
      .speakerSampleFormat = XMC_SAMPLE_LINEAR_PCM_S16_MONO,
      .speakerSampleRateHz = 22050,
      .speakerLatencySamples = 1024,
  };
  return config;
}

void xmc_appMain() {
  AppConfig cfg = xmc_appGetConfig();
  xmc_sysInit();
  xmc_displayInit(cfg.displayPixelFormat, 0);
  xmc_inputInit();
  xmc_speakerInit(cfg.speakerSampleFormat, cfg.speakerSampleRateHz,
                  cfg.speakerLatencySamples, NULL);
  xmc_appSetup();
  while (1) {
    xmc_sysService();
    xmc_appLoop();
  }
}
