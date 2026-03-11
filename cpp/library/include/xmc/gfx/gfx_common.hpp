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
};

static inline uint16_t blend4444(uint16_t a, uint16_t b, int32_t alpha) {
  if (alpha <= 0) {
    return a;
  } else if (alpha >= 256) {
    return b;
  } else {
    uint32_t inv_alpha = 256 - alpha;
    uint32_t ca = (((a & 0xF000) * inv_alpha + (b & 0xF000) * alpha) >> 8) & 0xF000;
    uint32_t cr = (((a & 0x0F00) * inv_alpha + (b & 0x0F00) * alpha) >> 8) & 0x0F00;
    uint32_t cg = (((a & 0x00F0) * inv_alpha + (b & 0x00F0) * alpha) >> 8) & 0x00F0;
    uint32_t cb = (((a & 0x000F) * inv_alpha + (b & 0x000F) * alpha) >> 8) & 0x000F;
    return static_cast<uint16_t>(ca | cr | cg | cb);
  }
}

}  // namespace xmc

#endif  // XMC_GFX_COMMON_HPP
