/**
 * @file app.h
 * @brief User application interface
 */

#ifndef XMC_APP_H
#define XMC_APP_H

#include "xmc/audio_common.h"
#include "xmc/display.h"
#include "xmc/xmc_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
  xmc_display_intf_format_t displayPixelFormat;
  xmc_audio_sample_format_t speakerSampleFormat;
  uint32_t speakerSampleRateHz;
  uint32_t speakerLatencySamples;

} AppConfig;

/**
 * Get the default application configuration.
 * @return A default application configuration struct.
 */
AppConfig xmcGetDefaultAppConfig();

/**
 * Get application configuration parameters. This function will be called before
 * xmc_appSetup, and the returned configuration will be used to initialize the
 * application. You can use this function to specify parameters such as speaker
 * sample rate, display resolution, etc.
 */
AppConfig xmc_appGetConfig();

/**
 * User defined setup function. This will be called once at the beginning of the
 * program. You can use this function to initialize your application, set up
 * peripherals, etc.
 */
void xmc_appSetup();

/**
 * User defined loop function. This will be called repeatedly after
 * xmc_appSetup. You can use this function to implement the main logic of
 * your application.
 */
void xmc_appLoop();

#if defined(__cplusplus)
}
#endif

#endif
