/**
 * @file sprite565.hpp
 * @brief Sprite class for 16-bit RGB565 images
 */

#ifndef XMC_SPRITE565_HPP
#define XMC_SPRITE565_HPP

#include "xmc/gfx/sprite.hpp"

namespace xmc {

/** Sprite class for 16-bit RGB565 images */
class Sprite565Class : public SpriteClass {
 public:
  Sprite565Class(int width, int height, uint32_t stride, void *data,
                 bool auto_free = false)
      : SpriteClass(pixel_format_t::RGB565, width, height, stride, data,
                    auto_free) {}

  Sprite565Class(int width, int height, xmc_ram_cap_t caps = XMC_RAM_CAP_DMA)
      : SpriteClass(pixel_format_t::RGB565, width, height,
                    sizeof(uint16_t) * width,
                    xmc_malloc(sizeof(uint16_t) * width * height, caps), true) {
  }

 protected:
  void on_set_pixel(int x, int y, uint16_t color) override;
  uint16_t on_get_pixel(int x, int y) const override;
  void on_fill_rect(int x, int y, int w, int h, uint16_t color) override;
  void on_draw_image(const Sprite &image, int dx, int dy, int w, int h,
                     int sx, int sy) override;
  xmc_status_t on_start_transfer_to_display(int dx, int dy, int sy,
                                            int h) override;
};

static inline Sprite create_sprite565(int width, int height,
                                     xmc_ram_cap_t caps = XMC_RAM_CAP_DMA) {
  return std::make_shared<Sprite565Class>(width, height, caps);
}

static inline Sprite create_sprite565(int width, int height, uint32_t stride,
                                     void *data, bool auto_free = false) {
  return std::make_shared<Sprite565Class>(width, height, stride, data,
                                          auto_free);
}

}  // namespace xmc

#endif
