#include "xmc/hw/dma_irq.h"

#include <hardware/dma.h>
#include <stddef.h>

typedef struct {
  DmaIrqHandlerCb handlerFast;
  DmaIrqHandlerCb handlerSlow;
  void *context;
} DmaHandlerEntry;

DmaHandlerEntry contexts[16] = {0};

void xmc_dmaRegisterIrqHandler(int dmaCh,
                                  DmaIrqHandlerCb handlerFast,
                                  DmaIrqHandlerCb handlerSlow,
                                  void *context) {
  contexts[dmaCh].handlerFast = handlerFast;
  contexts[dmaCh].handlerSlow = handlerSlow;
  contexts[dmaCh].context = context;
}

void xmc_dmaUnregisterIrqHandler(int dmaCh) {
  contexts[dmaCh] = (DmaHandlerEntry){0};
}

void xmc_dmaIrqHandler(void) {
  uint32_t ints0 = dma_hw->ints0;
  uint32_t tmp = ints0;

  int dmaCh;
  do {
    dmaCh = __builtin_ctz(tmp);
    if (dmaCh >= 16) {
      break;
    }
    tmp &= ~(1u << dmaCh);

    DmaHandlerEntry *entry = &contexts[dmaCh];
    if (entry->handlerFast) {
      entry->handlerFast(entry->context);
    }
  } while (tmp);

  tmp = ints0;
  do {
    dmaCh = __builtin_ctz(tmp);
    if (dmaCh >= 16) {
      break;
    }
    tmp &= ~(1u << dmaCh);

    DmaHandlerEntry *entry = &contexts[dmaCh];
    if (entry->handlerSlow) {
      entry->handlerSlow(entry->context);
    }
  } while (tmp);

  dma_hw->ints0 = ints0;
}
