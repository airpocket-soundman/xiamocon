#include "xmc/gfx/sprite4444.hpp"
#include "xmc/display.h"

#include <string.h>

namespace xmc {

void Sprite4444::on_set_pixel(int x, int y, uint16_t color) {
  uint16_t *line = (uint16_t *)line_ptr(y);
  line[x] = color;
}

uint16_t Sprite4444::on_get_pixel(int x, int y) const {
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

xmc_status_t Sprite4444::on_start_transfer_to_display(int dx, int dy, int sy,
                                                      int h) {
  return XMC_ERR_DISPLAY_UNSUPPORTED_FORMAT;
}

}  // namespace xmc