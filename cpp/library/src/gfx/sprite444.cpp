#include "xmc/gfx/sprite444.hpp"
#include "xmc/display.h"

namespace xmc {

void Sprite444::set_pixel(int x, int y, uint16_t color) {
  if (x < 0 || x >= width || y < 0 || y >= height) return;
  uint8_t *line = (uint8_t *)line_ptr(y);

  int i = x / 2 * 3;
  if ((x & 1) == 0) {
    line[i + 0] = (color >> 4) & 0xFF;
    line[i + 1] = (line[i + 1] & 0x0F) | ((color & 0x0F) << 4);
  } else {
    line[i + 1] = (line[i + 1] & 0xF0) | ((color >> 8) & 0x0F);
    line[i + 2] = color & 0xFF;
  }
}

uint16_t Sprite444::get_pixel(int x, int y) const {
  if (x < 0 || x >= width || y < 0 || y >= height) return 0;
  uint8_t *line = (uint8_t *)line_ptr(y);

  int i = x / 2 * 3;
  if ((x & 1) == 0) {
    return ((uint16_t)line[i + 0] << 4) | (line[i + 1] >> 4);
  } else {
    return ((uint16_t)(line[i + 1] & 0x0F) << 8) | line[i + 2];
  }
}

void Sprite444::on_fill_rect(int x, int y, int width, int height,
                             uint16_t color) {
  if (width <= 0 || height <= 0) return;

  // todo: optimize
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      set_pixel(x + i, y + j, color);
    }
  }
}

xmc_status_t Sprite444::start_transfer_to_display(int x, int y) {
  XMC_ERR_RET(xmc_display_set_window(x, y, width, height));
  XMC_ERR_RET(xmc_display_write_pixels_start(data, stride * height, false));
  return XMC_OK;
}

}  // namespace xmc