#ifndef XMC_HW_DMA_IRQ_H
#define XMC_HW_DMA_IRQ_H

#include "xmc/hw/dma.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef void (*DmaIrqHandlerCb)(void *context);

void xmc_dmaRegisterIrqHandler(int dmaCh,
                                  DmaIrqHandlerCb handlerFast,
                                  DmaIrqHandlerCb handlerSlow,
                                  void *context);
void xmc_dmaUnregisterIrqHandler(int dmaCh);

void xmc_dmaIrqHandler(void);

#if defined(__cplusplus)
}
#endif

#endif
