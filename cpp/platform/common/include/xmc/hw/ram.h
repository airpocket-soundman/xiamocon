#ifndef XMC_HW_RAM_H
#define XMC_HW_RAM_H

#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    XMC_RAM_CAP_NONE = 0,
    XMC_RAM_CAP_DMA = 1 << 0,
} xmc_ram_cap_t;

#if defined(__cplusplus)
extern "C" {
#endif

void *xmc_malloc(size_t size, xmc_ram_cap_t caps);
void xmc_free(void *ptr);

#if defined(__cplusplus)
}
#endif

#endif
