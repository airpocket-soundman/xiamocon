#include "xmc/speaker.h"
#include "xmc/hw/pins.h"
#include "xmc/ioex.h"

SdacInst sdac;

XmcStatus xmc_speakerInit(xmc_audio_sample_format_t fmt, uint32_t rate,
                              uint32_t latency, float *actualRateHz) {
  xmc_audio_format_t sdac_fmt;
  SdacConfig cfg;
  sdac_fmt.sample_format = fmt;
  sdac_fmt.sampleRateHz = rate;
  cfg.format = sdac_fmt;
  cfg.latencySamples = latency;
  XMC_ERR_RET(xmc_ioexWrite(XMC_IOEX_PIN_SPEAKER_MUTE, 1));
  XMC_ERR_RET(xmc_ioexSetDir(XMC_IOEX_PIN_SPEAKER_MUTE, true));
  XMC_ERR_RET(xmc_sdac_init(&sdac, XMC_PIN_AUDIO_OUT, &cfg, actualRateHz));
  return XMC_OK;
}

XmcStatus xmc_speakerDeinit(void) { return xmc_sdacDeinit(&sdac); }

XmcStatus xmc_speakerSetMuted(bool muted) {
  return xmc_ioexWrite(XMC_IOEX_PIN_SPEAKER_MUTE, muted ? 1 : 0);
}

XmcStatus xmc_speakerSetSourcePort(xmc_audio_source_port_t *src) {
  return xmc_sdac_set_source(&sdac, src);
}

XmcStatus xmc_speakerService(void) { return xmc_sdac_service(&sdac); }
