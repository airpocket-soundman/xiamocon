#include "xmc/app.hpp"
#include "xmc/app_entry.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/input.hpp"
#include "xmc/speaker.hpp"
#include "xmc/system.hpp"

#include <stddef.h>

namespace xmc {

AppConfig getDefaultAppConfig() {
  AppConfig config = {
      .displayPixelFormat = display::InterfaceFormat::RGB565,
      .speakerSampleFormat = audio::SampleFormat::LINEAR_PCM_S16_MONO,
      .speakerSampleRateHz = 22050,
      .speakerLatencySamples = 1024,
  };
  return config;
}

void appMain() {
  AppConfig cfg = appGetConfig();
  system::init();
  display::init(cfg.displayPixelFormat, 0);
  input::init();
  speaker::init(cfg.speakerSampleFormat, cfg.speakerSampleRateHz,
                cfg.speakerLatencySamples, NULL);
  appSetup();
  while (1) {
    system::Service();
    appLoop();
  }
}

}  // namespace xmc
