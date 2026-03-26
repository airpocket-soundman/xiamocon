#include "xmc/hw/ram.hpp"

#include <stdlib.h>

void *xmcMalloc(size_t size, xmc_ram_cap_t caps) {
  (void)caps;
  return malloc(size);
}

void xmcFree(void *ptr) { free(ptr); }
