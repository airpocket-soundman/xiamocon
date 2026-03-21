#include "xmc/gfx/sprite444.hpp"
#include "xmc/display.h"
#include "xmc/geo.hpp"
#include "xmc/gfx/sprite4444.hpp"

#include <string.h>
#include <memory>

namespace xmc {

void Sprite444Class::on_set_pixel(int x, int y, uint16_t color) {
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

uint16_t Sprite444Class::on_get_pixel(int x, int y) const {
  uint8_t *line = (uint8_t *)line_ptr(y);
  int i = x / 2 * 3;
  if ((x & 1) == 0) {
    return ((uint16_t)line[i + 0] << 4) | (line[i + 1] >> 4);
  } else {
    return ((uint16_t)(line[i + 1] & 0x0F) << 8) | line[i + 2];
  }
}

void Sprite444Class::on_fill_rect(int x, int y, int width, int height,
                                  uint16_t color) {
  if (width <= 0 || height <= 0) return;

  for (int i = 0; i < width; i++) {
    on_set_pixel(x + i, y, color);
  }
  int copy_l = (x + 1) & 0xFFFFFFFE;
  int copy_r = (x + width) & 0xFFFFFFFE;
  int copy_offset = copy_l * 3 / 2;
  int copy_bytes = (copy_r - copy_l) * 3 / 2;
  uint8_t *copy_src = (uint8_t *)line_ptr(y) + copy_offset;
  for (int j = 1; j < height; j++) {
    uint8_t *copy_dst = (uint8_t *)line_ptr(y + j) + copy_offset;
    if (copy_bytes > 0) {
      memcpy(copy_dst, copy_src, copy_bytes);
    }
    if (x & 1) {
      on_set_pixel(x, y + j, color);
    }
    if (((x + width) & 1)) {
      on_set_pixel(x + width - 1, y + j, color);
    }
  }
}

void Sprite444Class::on_draw_image(const Sprite &image, int dx, int dy, int w,
                                   int h, int sx, int sy) {
  switch (image->format()) {
    case pixel_format_t::ARGB4444:
      // todo: optimize
      for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
          uint16_t c = image->get_pixel(sx + i, sy + j);
          if ((c & 0xF000) == 0) continue;
          on_set_pixel(dx + i, dy + j, c);
        }
      }
      break;

    default:
      // todo: implement
      break;
  }
}

xmc_status_t Sprite444Class::on_start_transfer_to_display(int dx, int dy,
                                                          int sy, int h) {
  XMC_ERR_RET(xmc_display_set_window(dx, dy, width_, h));
  XMC_ERR_RET(xmc_display_write_pixels_start(line_ptr(sy), stride_ * h, false));
  return XMC_OK;
}

}  // namespace xmc
