#include "xmc/hw/ram.h"

#include <pico/stdlib.h>

void *xmcMalloc(size_t size, xmc_ram_cap_t caps) { return malloc(size); }
void xmcFree(void *ptr) { free(ptr); }
