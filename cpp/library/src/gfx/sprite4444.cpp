#include "xmc/gfx/sprite4444.hpp"
#include "xmc/display.hpp"

#include <string.h>

namespace xmc {

void Sprite4444Class::onSetPixel(int x, int y, uint16_t color) {
  uint16_t *line = (uint16_t *)linePtr(y);
  line[x] = color;
}

uint16_t Sprite4444Class::onGetPixel(int x, int y) const {
  uint16_t *line = (uint16_t *)linePtr(y);
  return line[x];
}

void Sprite4444Class::onFillRect(int x, int y, int width, int height,
                                 uint16_t color) {
  if (width <= 0 || height <= 0) return;
  for (int j = 0; j < height; j++) {
    uint16_t *line = (uint16_t *)linePtr(y + j);
    for (int i = 0; i < width; i++) {
      line[x + i] = color;
    }
  }
}

void Sprite4444Class::onDrawImage(const Sprite &image, int dx, int dy, int w,
                                  int h, int sx, int sy) {
  switch (image->format) {
    default:
      // todo: implement
      break;
  }
}

XmcStatus Sprite4444Class::onStartTransferToDisplay(int dx, int dy, int sy,
                                                        int h) {
  return XMC_ERR_DISPLAY_UNSUPPORTED_FORMAT;
}

}  // namespace xmc