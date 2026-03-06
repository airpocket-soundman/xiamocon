#ifndef XMC_HW_HW_COMMON_H
#define XMC_HW_HW_COMMON_H

#include "xmc/xmc_common.h"

/**
 * DMA transfer configuration.
 */
typedef struct {
  /** Pointer to the data buffer. */
  void *ptr;
  /** Size of each element in bytes. */
  int element_size;
  /** Number of elements to transfer. */
  int length;
  /** Whether to increment the pointer after each transfer. */
  bool increment;
} xmc_dma_config_t;

#endif
