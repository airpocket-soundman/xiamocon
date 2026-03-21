#include "xmc/app.h"
#include "xmc/app_entry.h"
#include "xmc/hw/timer.h"
#include "xmc/input.h"
#include "xmc/speaker.h"
#include "xmc/system.h"

#include <stddef.h>

xmc_app_config_t xmc_get_default_app_config() {
  xmc_app_config_t config = {
      .display_pixel_format = XMC_DISP_INTF_FORMAT_RGB565,
      .speaker_sample_format = XMC_SAMPLE_LINEAR_PCM_S16_MONO,
      .speaker_sample_rate_hz = 22050,
      .speaker_latency_samples = 1024,
  };
  return config;
}

void xmc_app_main() {
  xmc_app_config_t cfg = xmc_app_get_config();
  xmc_sys_init();
  xmc_display_init(cfg.display_pixel_format, 0);
  xmc_input_init();
  xmc_speaker_init(cfg.speaker_sample_format, cfg.speaker_sample_rate_hz,
                   cfg.speaker_latency_samples, NULL);
  xmc_app_setup();
  while (1) {
    xmc_sys_service();
    xmc_app_loop();
  }
}
