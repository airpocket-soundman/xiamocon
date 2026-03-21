#include "xmc/audio/mixer.hpp"

namespace xmc {

static void xmc_mixerRequestData(void *buffer, uint32_t numSamples,
                                   void *context);

Mixer::Mixer(int numSources) : numSources(numSources) {
  sources = new xmc_audio_source_port_t *[numSources];
  for (int i = 0; i < numSources; i++) {
    sources[i] = nullptr;
  }
  output.requestData = xmc_mixerRequestData;
  output.context = this;
}

Mixer::~Mixer() { delete[] sources; }

void Mixer::render(int16_t *buffer, uint32_t numSamples) {
  for (int i = 0; i < numSources; i++) {
    if (sources[i]) {
      sources[i]->requestData(buffer, numSamples, sources[i]->context);
    }
  }
}

static void xmc_mixerRequestData(void *buffer, uint32_t numSamples,
                                   void *context) {
  ((Mixer *)context)->render((int16_t *)buffer, numSamples);
}

}  // namespace xmc
