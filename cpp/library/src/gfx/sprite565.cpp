#include "xmc/gfx/sprite565.hpp"
#include "xmc/display.h"

#include <string.h>

namespace xmc {

void Sprite565Class::on_set_pixel(int x, int y, uint16_t color) {
  uint16_t *line = (uint16_t *)line_ptr(y);
  line[x] = color;
}

uint16_t Sprite565Class::on_get_pixel(int x, int y) const {
  uint16_t *line = (uint16_t *)line_ptr(y);
  return line[x];
}

void Sprite565Class::on_fill_rect(int x, int y, int width, int height,
                                  uint16_t color) {
  if (width <= 0 || height <= 0) return;
  for (int j = 0; j < height; j++) {
    uint16_t *line = (uint16_t *)line_ptr(y + j);
    for (int i = 0; i < width; i++) {
      line[x + i] = color;
    }
  }
}

void Sprite565Class::on_draw_image(const Sprite &image, int dx, int dy, int w,
                                   int h, int sx, int sy) {
  switch (image->format()) {
    default:
      // todo: implement
      break;
  }
}

xmc_status_t Sprite565Class::on_start_transfer_to_display(int dx, int dy,
                                                          int sy, int h) {
  XMC_ERR_RET(xmc_display_set_window(dx, dy, width_, h));
  XMC_ERR_RET(xmc_display_write_pixels_start(line_ptr(sy), stride_ * h, false));
  return XMC_OK;
}

}  // namespace xmc