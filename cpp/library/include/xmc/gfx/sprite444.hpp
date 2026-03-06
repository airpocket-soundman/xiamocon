#ifndef XMC_SPRITE444_HPP
#define XMC_SPRITE444_HPP

#include "xmc/gfx/sprite.hpp"

namespace xmc {

static inline uint32_t stride444(int width) { return (width * 12 + 7) / 8; }

class Sprite444 : public Sprite<pixel_format_t::RGB444, uint16_t> {
 public:
  Sprite444(int width, int height, xmc_ram_cap_t caps = XMC_RAM_CAP_DMA)
      : Sprite<pixel_format_t::RGB444, uint16_t>(
            width, height, stride444(width),
            xmc_malloc(stride444(width) * height, caps), true) {}

  void set_pixel(int x, int y, uint16_t color) override;
  uint16_t get_pixel(int x, int y) const override;
  void on_fill_rect(int x, int y, int w, int h, uint16_t color) override;
  xmc_status_t start_transfer_to_display(int x, int y) override;
};

}  // namespace xmc

#endif
