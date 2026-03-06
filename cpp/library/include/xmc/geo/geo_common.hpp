/**
 * @file geo_common.hpp
 * @brief Common geometric utilities
 */

#ifndef XMC_GEO_GEO_COMMON_HPP
#define XMC_GEO_GEO_COMMON_HPP

#include "xmc/xmc_common.h"

namespace xmc {

/**
 * Clip a rectangle to fit within a maximum width and height. The rectangle is
 * defined by its top-left corner (x, y) and its width and height (w, h). The
 * maximum width and height are defined by max_w and max_h. The function
 * modifies the rectangle in place to ensure that it fits within the specified
 * bounds. If the rectangle is completely outside the bounds, it will be clipped
 * to a zero-size rectangle.
 */
void clip_rect(int *x, int *y, int *w, int *h, int max_w, int max_h);

}  // namespace xmc

#endif
