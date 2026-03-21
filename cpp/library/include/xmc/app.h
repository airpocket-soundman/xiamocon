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
  xmc_display_intf_format_t display_pixel_format;
  xmc_audio_sample_format_t speaker_sample_format;
  uint32_t speaker_sample_rate_hz;
  uint32_t speaker_latency_samples;

} xmc_app_config_t;

/**
 * Get the default application configuration.
 * @return A default application configuration struct.
 */
xmc_app_config_t xmc_get_default_app_config();

/**
 * Get application configuration parameters. This function will be called before
 * xmc_app_setup, and the returned configuration will be used to initialize the
 * application. You can use this function to specify parameters such as speaker
 * sample rate, display resolution, etc.
 */
xmc_app_config_t xmc_app_get_config();

/**
 * User defined setup function. This will be called once at the beginning of the
 * program. You can use this function to initialize your application, set up
 * peripherals, etc.
 */
void xmc_app_setup();

/**
 * User defined loop function. This will be called repeatedly after
 * xmc_app_setup. You can use this function to implement the main logic of
 * your application.
 */
void xmc_app_loop();

#if defined(__cplusplus)
}
#endif

#endif
