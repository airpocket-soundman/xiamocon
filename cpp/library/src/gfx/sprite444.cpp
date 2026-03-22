#include "xmc/gfx/sprite444.hpp"
#include "xmc/display.hpp"
#include "xmc/geo.hpp"
#include "xmc/gfx/sprite4444.hpp"

#include <string.h>
#include <memory>

namespace xmc {

void Sprite444Class::onSetPixel(int x, int y, uint16_t color) {
  uint8_t *line = (uint8_t *)linePtr(y);
  int i = x / 2 * 3;
  if ((x & 1) == 0) {
    line[i + 0] = (color >> 4) & 0xFF;
    line[i + 1] = (line[i + 1] & 0x0F) | ((color & 0x0F) << 4);
  } else {
    line[i + 1] = (line[i + 1] & 0xF0) | ((color >> 8) & 0x0F);
    line[i + 2] = color & 0xFF;
  }
}

uint16_t Sprite444Class::onGetPixel(int x, int y) const {
  uint8_t *line = (uint8_t *)linePtr(y);
  int i = x / 2 * 3;
  if ((x & 1) == 0) {
    return ((uint16_t)line[i + 0] << 4) | (line[i + 1] >> 4);
  } else {
    return ((uint16_t)(line[i + 1] & 0x0F) << 8) | line[i + 2];
  }
}

void Sprite444Class::onFillRect(int x, int y, int width, int height,
                                uint16_t color) {
  if (width <= 0 || height <= 0) return;

  for (int i = 0; i < width; i++) {
    onSetPixel(x + i, y, color);
  }
  int copyL = (x + 1) & 0xFFFFFFFE;
  int copyR = (x + width) & 0xFFFFFFFE;
  int copyOffset = copyL * 3 / 2;
  int copyBytes = (copyR - copyL) * 3 / 2;
  uint8_t *copySrc = (uint8_t *)linePtr(y) + copyOffset;
  for (int j = 1; j < height; j++) {
    uint8_t *copyDst = (uint8_t *)linePtr(y + j) + copyOffset;
    if (copyBytes > 0) {
      memcpy(copyDst, copySrc, copyBytes);
    }
    if (x & 1) {
      onSetPixel(x, y + j, color);
    }
    if (((x + width) & 1)) {
      onSetPixel(x + width - 1, y + j, color);
    }
  }
}

void Sprite444Class::onDrawImage(const Sprite &image, int dx, int dy, int w,
                                 int h, int sx, int sy) {
  switch (image->format) {
    case pixel_format_t::ARGB4444:
      // todo: optimize
      for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
          uint16_t c = image->getPixel(sx + i, sy + j);
          if ((c & 0xF000) == 0) continue;
          onSetPixel(dx + i, dy + j, c);
        }
      }
      break;

    default:
      // todo: implement
      break;
  }
}

XmcStatus Sprite444Class::onStartTransferToDisplay(int dx, int dy, int sy,
                                                   int h) {
  XMC_ERR_RET(display::setWindow(dx, dy, width, h));
  XMC_ERR_RET(display::writePixelsStart(linePtr(sy), stride * h, false));
  return XMC_OK;
}

}  // namespace xmc
