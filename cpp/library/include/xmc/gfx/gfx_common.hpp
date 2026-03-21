#ifndef XMC_GFX_COMMON_HPP
#define XMC_GFX_COMMON_HPP

#include "xmc/xmc_common.h"

#include <stdint.h>

namespace xmc {

/** Pixel formats for sprites and display interfaces. */
enum class pixel_format_t {
  /** 12-bit RGB format (4 bits per channel) */
  RGB444,
  /** 16-bit ARGB format (4 bits per channel) */
  ARGB4444,
  /** 16-bit RGB format (5 bits for red and blue, 6 bits for green) */
  RGB565,
};

using raw_color = uint16_t;

static inline uint16_t blend4444(uint16_t a, uint16_t b, int32_t alpha) {
  if (alpha <= 0) {
    return a;
  } else if (alpha >= 256) {
    return b;
  } else {
    uint32_t inv_alpha = 256 - alpha;
    uint32_t ca =
        (((a & 0xF000) * inv_alpha + (b & 0xF000) * alpha) >> 8) & 0xF000;
    uint32_t cr =
        (((a & 0x0F00) * inv_alpha + (b & 0x0F00) * alpha) >> 8) & 0x0F00;
    uint32_t cg =
        (((a & 0x00F0) * inv_alpha + (b & 0x00F0) * alpha) >> 8) & 0x00F0;
    uint32_t cb =
        (((a & 0x000F) * inv_alpha + (b & 0x000F) * alpha) >> 8) & 0x000F;
    return static_cast<uint16_t>(ca | cr | cg | cb);
  }
}

union color4444 {
  uint16_t packed;
  struct {
    uint8_t b : 4;
    uint8_t g : 4;
    uint8_t r : 4;
    uint8_t a : 4;
  };

  color4444() : packed(0) {}
  color4444(uint16_t packed) : packed(packed) {}
  color4444(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
      : b(b), g(g), r(r), a(a) {}
  color4444(int a, int r, int g, int b)
      : b(b < 0 ? 0 : (b > 15 ? 15 : b)),
        g(g < 0 ? 0 : (g > 15 ? 15 : g)),
        r(r < 0 ? 0 : (r > 15 ? 15 : r)),
        a(a < 0 ? 0 : (a > 15 ? 15 : a)) {}
};

static inline uint16_t rgb565(int r, int g, int b) {
  if (r < 0) {
    r = 0;
  } else if (r > 31) {
    r = 31;
  }
  if (g < 0) {
    g = 0;
  } else if (g > 63) {
    g = 63;
  }
  if (b < 0) {
    b = 0;
  } else if (b > 31) {
    b = 31;
  }
  return (r << 3) | ((g & 0x38) >> 3) | ((g & 0x07) << 13) | (b << 8);
}

}  // namespace xmc

#endif  // XMC_GFX_COMMON_HPP
