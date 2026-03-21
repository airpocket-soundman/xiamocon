#include "xmc/geo/geo_common.hpp"

namespace xmc {

void clipRect(int *x, int *y, int *w, int *h, int maxW, int maxH) {
  if (*x < 0) {
    *w += *x;
    *x = 0;
  }
  if (*y < 0) {
    *h += *y;
    *y = 0;
  }
  if (*x + *w > maxW) {
    *w = maxW - *x;
  }
  if (*y + *h > maxH) {
    *h = maxH - *y;
  }
}

}  // namespace xmc
