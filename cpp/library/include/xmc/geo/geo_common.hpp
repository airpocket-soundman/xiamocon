#ifndef XMC_GEO_GEO_COMMON_HPP
#define XMC_GEO_GEO_COMMON_HPP

#include "xmc/xmc_common.h"

namespace xmc {
void clip_rect(int *x, int *y, int *w, int *h, int max_w, int max_h);
}

#endif
