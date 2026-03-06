#ifndef XMC_GEO_RECT_HPP
#define XMC_GEO_RECT_HPP

#include "xmc/geo/geo_common.hpp"
namespace xmc {

struct rect_t {
  int x;
  int y;
  int width;
  int height;

  inline bool contains(int px, int py) const {
    return px >= x && px < x + width && py >= y && py < y + height;
  }

  inline int right() const { return x + width; }
  inline int bottom() const { return y + height; }

  inline rect_t intersect(const rect_t& other) const {
    rect_t result;
    result.x = x > other.x ? x : other.x;
    result.y = y > other.y ? y : other.y;
    int r = right() < other.right() ? right() : other.right();
    int b = bottom() < other.bottom() ? bottom() : other.bottom();
    result.width = r - result.x;
    result.height = b - result.y;
    if (result.width < 0) {
      result.width = 0;
    }
    if (result.height < 0) {
      result.height = 0;
    }
    return result;
  }
};
}  // namespace xmc

#endif
