#include "xmc/hw/streaming_dac.hpp"

// Stub implementation: AtomS3 has no speaker output.

namespace xmc::audio {

SampleFormat sdacGetSupportedFormats(void) {
  return SampleFormat::LINEAR_PCM_S16_MONO;
}

StreamingDac::StreamingDac(int pin) : pin(pin) {
  handle = (void *)1;  // non-null sentinel
}

StreamingDac::~StreamingDac() { handle = nullptr; }

XmcStatus StreamingDac::start(const SdacConfig &cfg, float *actualRateHz) {
  if (actualRateHz) *actualRateHz = (float)cfg.format.rateHz;
  return XMC_OK;
}

XmcStatus StreamingDac::stop() { return XMC_OK; }

XmcStatus StreamingDac::setSource(SourcePort *src) {
  if (src) this->source = *src;
  return XMC_OK;
}

XmcStatus StreamingDac::service() { return XMC_OK; }

}  // namespace xmc::audio
