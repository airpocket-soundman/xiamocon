/**
 * @file sprite444.hpp
 * @brief Sprite class for 12-bit RGB444 images
 */

#ifndef XMC_SPRITE444_HPP
#define XMC_SPRITE444_HPP

#include "xmc/gfx/sprite.hpp"

namespace xmc {

/** Calculate the stride (in bytes) for a 12-bit RGB444 image */
static inline uint32_t stride444(int width) { return (width * 12 + 7) / 8; }

/** Sprite class for 12-bit RGB444 images */
class Sprite444Class : public SpriteClass {
 public:
  Sprite444Class(int width, int height, uint32_t stride, void *data,
                 bool autoFree = false)
      : SpriteClass(pixel_format_t::RGB444, width, height, stride444(width),
                        data, autoFree) {}

  Sprite444Class(int width, int height, xmc_ram_cap_t caps = XMC_RAM_CAP_DMA)
      : SpriteClass(pixel_format_t::RGB444, width, height, stride444(width),
                        xmcMalloc(stride444(width) * height, caps), true) {}

 protected:
  void onSetPixel(int x, int y, uint16_t color) override;
  uint16_t onGetPixel(int x, int y) const override;
  void onFillRect(int x, int y, int w, int h, uint16_t color) override;
  void onDrawImage(const Sprite &image, int dx, int dy, int w, int h,
                     int sx, int sy) override;
  XmcStatus on_start_transfer_to_display(int dx, int dy, int sy,
                                            int h) override;
};

static inline Sprite createSprite444(int width, int height,
                                     xmc_ram_cap_t caps = XMC_RAM_CAP_DMA) {
  return std::make_shared<Sprite444Class>(width, height, caps);
}

static inline Sprite createSprite444(int width, int height, uint32_t stride,
                                     void *data, bool autoFree = false) {
  return std::make_shared<Sprite444Class>(width, height, stride, data, autoFree);
}

}  // namespace xmc

#endif
