#ifndef XMC_DISPLAY_HPP
#define XMC_DISPLAY_HPP

#include "xmc/xmc_common.h"

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
  XMC_DISP_INTF_FORMAT_RGB444,
  XMC_DISP_INTF_FORMAT_RGB565,
  XMC_DISP_INTF_FORMAT_RGB666,
} xmc_display_intf_format_t;

#define XMC_DISPLAY_WIDTH 240
#define XMC_DISPLAY_HEIGHT 240

xmc_status_t xmc_display_init(xmc_display_intf_format_t format, int rotation);
xmc_status_t xmc_display_deinit();
xmc_status_t xmc_display_clear(uint32_t color);
xmc_status_t xmc_display_fill_rect(int x, int y, int width, int height,
                                   uint32_t color);
xmc_status_t xmc_display_set_window(int x, int y, int width, int height);
xmc_status_t xmc_display_write_pixels_start(const void *data,
                                            uint32_t num_bytes, bool repeated);
xmc_status_t xmc_display_write_pixels_complete();
xmc_status_t xmc_display_write_command(const uint8_t cmd, const uint8_t *params,
                                       uint32_t num_params);

static inline xmc_status_t xmc_display_write_command_0p(uint8_t cmd) {
  return xmc_display_write_command(cmd, NULL, 0);
}

static inline xmc_status_t xmc_display_write_command_1p(uint8_t cmd,
                                                        uint8_t data0) {
  return xmc_display_write_command(cmd, &data0, 1);
}

static inline xmc_status_t xmc_display_write_command_2p(uint8_t cmd,
                                                        uint8_t data0,
                                                        uint8_t data1) {
  uint8_t params[2] = {data0, data1};
  return xmc_display_write_command(cmd, params, 2);
}

static inline xmc_status_t xmc_display_write_command_3p(uint8_t cmd,
                                                        uint8_t data0,
                                                        uint8_t data1,
                                                        uint8_t data2) {
  uint8_t params[3] = {data0, data1, data2};
  return xmc_display_write_command(cmd, params, 3);
}

static inline xmc_status_t xmc_display_write_command_4p(
    uint8_t cmd, uint8_t data0, uint8_t data1, uint8_t data2, uint8_t data3) {
  uint8_t params[4] = {data0, data1, data2, data3};
  return xmc_display_write_command(cmd, params, 4);
}

#if defined(__cplusplus)
}
#endif

#endif
