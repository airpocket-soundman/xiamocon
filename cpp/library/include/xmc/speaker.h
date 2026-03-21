/**
 * @file speaker.h
 * @brief Speaker driver.
 */

#ifndef XMC_SPEAKER_H
#define XMC_SPEAKER_H

#include "xmc/hw/sdac.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Returns the supported audio sample formats for the speaker. This function is
 * a wrapper around xmc_sdacGetSupportedFormats() since the speaker uses the
 * SDAC as its underlying audio output.
 * @return A bitmask of supported audio sample formats for the speaker.
 */
static inline xmc_audio_sample_format_t xmc_speakerGetSupportedFormats(
    void) {
  return xmc_sdacGetSupportedFormats();
}

/**
 * Initializes the speaker with the given audio format, sample rate, and
 * latency. This function will set up the underlying SDAC instance with the
 * specified configuration and prepare the speaker for use.
 * @param format The audio sample format to use for the speaker. This must be
 * one of the formats returned by xmc_speakerGetSupportedFormats().
 * @param sampleRateHz The sample rate in Hz to use for the speaker.
 * @param latencySamples The latency in samples for the speaker.
 * @param actualRateHz A pointer to a variable that will receive the actual
 * sample rate in Hz that the speaker will use.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
XmcStatus xmc_speakerInit(xmc_audio_sample_format_t format,
                              uint32_t sampleRateHz, uint32_t latencySamples,
                              float *actualRateHz);

/**
 * Deinitializes the speaker. This function will free any resources allocated
 * during initialization and reset the speaker hardware to a safe state.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
XmcStatus xmc_speakerDeinit(void);

/**
 * Sets the muted state of the speaker. This function will mute or unmute the
 * speaker based on the provided boolean value.
 * @param muted A boolean value indicating whether to mute (true) or unmute
 * (false) the speaker.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
XmcStatus xmc_speakerSetMuted(bool muted);

/**
 * Sets the audio source port for the speaker. This function will configure the
 * speaker to use the specified audio source port for audio data.
 * @param src A pointer to the audio source port to use for the speaker.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
XmcStatus xmc_speakerSetSourcePort(xmc_audio_source_port_t *src);

/**
 * Services the speaker. This function should be called periodically to ensure
 * that the speaker continues to receive audio data from the audio source and to
 * handle any necessary processing or buffering. This function is a wrapper
 * around xmc_sdac_service() since the speaker uses the SDAC as its underlying
 * audio output.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
XmcStatus xmc_speakerService(void);

#if defined(__cplusplus)
}
#endif

#endif
