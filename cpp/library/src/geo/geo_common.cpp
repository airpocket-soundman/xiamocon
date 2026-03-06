#include "xmc/geo/geo_common.hpp"

namespace xmc {

void clip_rect(int *x, int *y, int *w, int *h, int max_w, int max_h) {
  if (*x < 0) {
    *w += *x;
    *x = 0;
  }
  if (*y < 0) {
    *h += *y;
    *y = 0;
  }
  if (*x + *w > max_w) {
    *w = max_w - *x;
  }
  if (*y + *h > max_h) {
    *h = max_h - *y;
  }
}

}  // namespace xmc
