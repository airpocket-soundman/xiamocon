#include "xmc/app.h"

xmc_app_config_t xmc_get_default_app_config() {
  xmc_app_config_t config = {
      .speaker_sample_format = XMC_SAMPLE_LINEAR_PCM_S16_MONO,
      .speaker_sample_rate_hz = 22050,
      .speaker_latency_samples = 1024,
  };
  return config;
}
