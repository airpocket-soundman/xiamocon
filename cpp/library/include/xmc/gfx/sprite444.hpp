#ifndef XMC_SPRITE444_HPP
#define XMC_SPRITE444_HPP

#include "xmc/gfx/sprite.hpp"

namespace xmc {

/** Calculate the stride (in bytes) for a 12-bit RGB444 image */
static inline uint32_t stride444(int width) { return (width * 12 + 7) / 8; }

/** Sprite class for 12-bit RGB444 images */
class Sprite444 : public Sprite<pixel_format_t::RGB444, uint16_t> {
 public:
  Sprite444(int width, int height, xmc_ram_cap_t caps = XMC_RAM_CAP_DMA)
      : Sprite<pixel_format_t::RGB444, uint16_t>(
            width, height, stride444(width),
            xmc_malloc(stride444(width) * height, caps), true) {}


  xmc_status_t draw_image(const Sprite4444& image, int dx, int dy, int w, int h,
                          int sx, int sy);

 protected:
  void on_set_pixel(int x, int y, uint16_t color) override;
  uint16_t on_get_pixel(int x, int y) const override;
  void on_fill_rect(int x, int y, int w, int h, uint16_t color) override;
  xmc_status_t on_start_transfer_to_display(int dx, int dy, int sy, int h) override;
};

}  // namespace xmc

#endif
