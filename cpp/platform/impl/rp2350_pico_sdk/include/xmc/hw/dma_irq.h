#ifndef XMC_HW_DMA_IRQ_H
#define XMC_HW_DMA_IRQ_H

#include "xmc/hw/dma.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef void (*xmc_dma_irq_handler_t)(void *context);

void xmc_dma_register_irq_handler(int dma_ch,
                                  xmc_dma_irq_handler_t handler_fast,
                                  xmc_dma_irq_handler_t handler_slow,
                                  void *context);
void xmc_dma_unregister_irq_handler(int dma_ch);

void xmc_dma_irq_handler(void);

#if defined(__cplusplus)
}
#endif

#endif
