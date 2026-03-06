#ifndef XMC_HW_HW_COMMON_H
#define XMC_HW_HW_COMMON_H

#include "xmc/xmc_common.h"

typedef struct {
  void *ptr;
  int element_size;
  int length;
  bool increment;
} xmc_dma_config_t;

#endif
