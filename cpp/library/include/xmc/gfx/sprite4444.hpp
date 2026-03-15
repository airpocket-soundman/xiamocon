#ifndef XMC_SPRITE4444_HPP
#define XMC_SPRITE4444_HPP

#include "xmc/gfx/sprite.hpp"

namespace xmc {

/** Sprite class for 16-bit ARGB4444 images */
class Sprite4444 : public Sprite<pixel_format_t::ARGB4444, uint16_t> {
 public:
  Sprite4444(int width, int height, xmc_ram_cap_t caps = XMC_RAM_CAP_DMA)
      : Sprite<pixel_format_t::ARGB4444, uint16_t>(
            width, height, width * sizeof(uint16_t),
            xmc_malloc(width * height * sizeof(uint16_t), caps), true) {}

  Sprite4444(int width, int height, void *data, bool auto_free = false)
      : Sprite<pixel_format_t::ARGB4444, uint16_t>(
            width, height, width * sizeof(uint16_t), data, auto_free) {}

 protected:
  void on_set_pixel(int x, int y, uint16_t color) override;
  uint16_t on_get_pixel(int x, int y) const override;
  void on_fill_rect(int x, int y, int w, int h, uint16_t color) override;
  xmc_status_t on_start_transfer_to_display(int dx, int dy, int sy,
                                            int h) override;
};

}  // namespace xmc

#endif
