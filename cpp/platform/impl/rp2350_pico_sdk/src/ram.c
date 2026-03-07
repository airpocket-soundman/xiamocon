#include "xmc/hw/ram.h"

#include <pico/stdlib.h>

void *xmc_malloc(size_t size, xmc_ram_cap_t caps) { return malloc(size); }
void xmc_free(void *ptr) { free(ptr); }
