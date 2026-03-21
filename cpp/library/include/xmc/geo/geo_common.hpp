/**
 * @file geo_common.hpp
 * @brief Common geometric utilities
 */

#ifndef XMC_GEO_GEO_COMMON_HPP
#define XMC_GEO_GEO_COMMON_HPP

#include "xmc/xmc_common.h"

namespace xmc {

enum class PrimitiveMode {
  POINTS,
  LINES,
  LINE_LOOP,
  LINE_STRIP,
  TRIANGLES,
  TRIANGLE_STRIP,
  TRIANGLE_FAN,
};

/**
 * Clip a rectangle to fit within a maximum width and height. The rectangle is
 * defined by its top-left corner (x, y) and its width and height (w, h). The
 * maximum width and height are defined by maxW and maxH. The function
 * modifies the rectangle in place to ensure that it fits within the specified
 * bounds. If the rectangle is completely outside the bounds, it will be clipped
 * to a zero-size rectangle.
 */
void clipRect(int *x, int *y, int *w, int *h, int maxW, int maxH);

}  // namespace xmc

#endif
