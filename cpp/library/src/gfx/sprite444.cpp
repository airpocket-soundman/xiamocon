#include "xmc/gfx/sprite444.hpp"
#include "xmc/display.h"
#include "xmc/geo.hpp"
#include "xmc/gfx/sprite4444.hpp"

#include <string.h>

namespace xmc {

void Sprite444::on_set_pixel(int x, int y, uint16_t color) {
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

uint16_t Sprite444::on_get_pixel(int x, int y) const {
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

xmc_status_t Sprite444::draw_image(const Sprite4444 &image, int dx, int dy,
                                   int w, int h, int sx, int sy) {
  rect_t view = {0, 0, width, height};
  rect_t dst = {dx, dy, w, h};
  dst = dst.intersect(view);
  if (dst.width <= 0 || dst.height <= 0) return XMC_OK;
  sx += dst.x - dx;
  sy += dst.y - dy;
  dx = dst.x;
  dy = dst.y;
  w = dst.width;
  h = dst.height;
  rect_t img = {0, 0, image.width, image.height};
  rect_t src = {sx, sy, w, h};
  src = src.intersect(img);
  if (src.width <= 0 || src.height <= 0) return XMC_OK;
  dx += src.x - sx;
  dy += src.y - sy;
  w = src.width;
  h = src.height;

  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      uint16_t c = image.get_pixel(sx + i, sy + j);
      if ((c & 0xF000) == 0) continue;
      on_set_pixel(dx + i, dy + j, c);
    }
  }
  return XMC_OK;
}

xmc_status_t Sprite444::on_start_transfer_to_display(int dx, int dy, int sy,
                                                     int h) {
  XMC_ERR_RET(xmc_display_set_window(dx, dy, width, h));
  XMC_ERR_RET(xmc_display_write_pixels_start((uint8_t *)data + stride * sy,
                                             stride * h, false));
  return XMC_OK;
}

}  // namespace xmc
