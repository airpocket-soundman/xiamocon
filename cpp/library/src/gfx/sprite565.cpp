#include "xmc/gfx/sprite565.hpp"
#include "xmc/display.h"

#include <string.h>

namespace xmc {

void Sprite565Class::onSetPixel(int x, int y, uint16_t color) {
  uint16_t *line = (uint16_t *)linePtr(y);
  line[x] = color;
}

uint16_t Sprite565Class::onGetPixel(int x, int y) const {
  uint16_t *line = (uint16_t *)linePtr(y);
  return line[x];
}

void Sprite565Class::onFillRect(int x, int y, int width, int height,
                                  uint16_t color) {
  if (width <= 0 || height <= 0) return;
  for (int j = 0; j < height; j++) {
    uint16_t *line = (uint16_t *)linePtr(y + j);
    for (int i = 0; i < width; i++) {
      line[x + i] = color;
    }
  }
}

void Sprite565Class::onDrawImage(const Sprite &image, int dx, int dy, int w,
                                   int h, int sx, int sy) {
  switch (image->format) {
    default:
      // todo: implement
      break;
  }
}

XmcStatus Sprite565Class::on_start_transfer_to_display(int dx, int dy,
                                                          int sy, int h) {
  XMC_ERR_RET(xmc_displaySetWindow(dx, dy, width, h));
  XMC_ERR_RET(xmc_displayWritePixelsStart(linePtr(sy), stride * h, false));
  return XMC_OK;
}

}  // namespace xmc