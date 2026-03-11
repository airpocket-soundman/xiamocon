#include "xmc/gfx/sprite4444.hpp"
#include "xmc/display.h"

#include <string.h>

namespace xmc {

void Sprite4444::set_pixel(int x, int y, uint16_t color) {
  if (x < 0 || x >= width || y < 0 || y >= height) return;
  uint16_t *line = (uint16_t *)line_ptr(y);
  line[x] = color;
}

uint16_t Sprite4444::get_pixel(int x, int y) const {
  if (x < 0 || x >= width || y < 0 || y >= height) return 0;
  uint16_t *line = (uint16_t *)line_ptr(y);
  return line[x];
}

void Sprite4444::on_fill_rect(int x, int y, int width, int height,
                              uint16_t color) {
  if (width <= 0 || height <= 0) return;
  for (int j = 0; j < height; j++) {
    uint16_t *line = (uint16_t *)line_ptr(y + j);
    for (int i = 0; i < width; i++) {
      line[x + i] = color;
    }
  }
}

xmc_status_t Sprite4444::start_transfer_to_display(int x, int y) {
  XMC_ERR_RET(xmc_display_set_window(x, y, width, height));
  XMC_ERR_RET(xmc_display_write_pixels_start(data, stride * height, false));
  return XMC_OK;
}

}  // namespace xmc