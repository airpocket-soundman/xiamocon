#ifndef XMC_GFX_COMMON_HPP
#define XMC_GFX_COMMON_HPP

#include "xmc/xmc_common.h"

#include <stdint.h>

namespace xmc {

/** Pixel formats for sprites and display interfaces. */
enum class pixel_format_t {
  /** 12-bit RGB format (4 bits per channel) */
  RGB444,
};

}  // namespace xmc

#endif  // XMC_GFX_COMMON_HPP
