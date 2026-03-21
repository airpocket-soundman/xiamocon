#include "xmc/hw/sdac.h"
#include "xmc/hw/dma_irq.h"
#include "xmc/hw/pins.h"
#include "xmc/hw/ram.h"

#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/irq.h>
#include <hardware/pio.h>
#include <pico/stdlib.h>
#include <string.h>

static const uint32_t PDM_MIN_PDM_FREQ_HZ = 3000000;

typedef struct {
  SdacConfig cfg;
  xmc_audio_source_port_t *source;
  PIO pio;
  int pioSm;
  int pioOffset;
  int dmaCh;
  int nextWriteBank;
  int nextReadBank;
  uint8_t *srcFmtBuff;
  int16_t *s16Buff;
  uint32_t *dmaBuff;
  // uint16_t ditherPhase;
  uint32_t extraOversample;
  int32_t pdmLastInput;
  int32_t pdmLastQt;
  int32_t pdmWork0;
  int32_t pdmWork1;
  // int32_t pdmWork2;
  // int32_t pdmWork3;
  uint32_t pdmLfsr;
} SdacHw;

static const uint16_t pioInsts[] = {
    0x6001,
};

static const struct pio_program pioProgram = {
    .instructions = pioInsts,
    .length = 1,
    .origin = -1,
};

static void startNextDma(SdacInst *inst);
static void fillBuffer(SdacInst *inst);
static void dmaHandlerFast(void *context);
static void dmaHandlerSlow(void *context);

static inline void updateLfsr(uint32_t *lfsr) {
  uint32_t bit =
      ((*lfsr >> 0) ^ (*lfsr >> 10) ^ (*lfsr >> 30) ^ (*lfsr >> 31)) & 1;
  *lfsr = (*lfsr >> 1) | (bit << 31);
}

#define SUPPORTED_FORMATS \
  (XMC_SAMPLE_LINEAR_PCM_U8_MONO | XMC_SAMPLE_LINEAR_PCM_S16_MONO)

xmc_audio_sample_format_t xmc_sdacGetSupportedFormats(void) {
  return SUPPORTED_FORMATS;
}

XmcStatus xmc_sdac_init(SdacInst *inst, int pin, const SdacConfig *cfg,
                        float *actualRateHz) {
  // todo: calculate actualRateHz based on cfg->format.sampleRateHz and
  // hardware capabilities
  *actualRateHz = cfg->format.sampleRateHz;

  SdacHw *hw = malloc(sizeof(SdacHw));
  if (!hw) {
    return XMC_ERR_RAM_ALLOC_FAILED;
  }
  inst->hw = hw;
  inst->pin = pin;

  hw->cfg = *cfg;
  hw->pdmWork0 = 0;
  hw->pdmWork1 = 0;
  // hw->pdmWork2 = 0;
  // hw->pdmWork3 = 0;
  hw->pdmLastInput = 0;
  hw->pdmLfsr = 0xFFFFFFFF;

  uint32_t sysClkFreq = clock_get_hz(clk_sys);
  uint32_t pdmClkFreq = hw->cfg.format.sampleRateHz * 32;
  hw->extraOversample = (PDM_MIN_PDM_FREQ_HZ + pdmClkFreq - 1) / pdmClkFreq;
  if (hw->extraOversample < 1) hw->extraOversample = 1;
  pdmClkFreq *= hw->extraOversample;

  if ((cfg->format.sample_format & SUPPORTED_FORMATS) == 0) {
    XMC_ERR_RET(XMC_ERR_SPEAKER_UNSUPPORTED_FORMAT);
  }

  if (cfg->format.sample_format != XMC_SAMPLE_LINEAR_PCM_S16_MONO) {
    int bytes_per_sample =
        xmc_audio_get_bytes_per_sample(hw->cfg.format.sample_format);
    hw->srcFmtBuff = malloc(hw->cfg.latencySamples * bytes_per_sample);
    if (!hw->srcFmtBuff) {
      xmc_sdacDeinit(inst);
      return XMC_ERR_RAM_ALLOC_FAILED;
    }
  } else {
    hw->srcFmtBuff = NULL;
  }

  hw->s16Buff = malloc(hw->cfg.latencySamples * sizeof(int16_t));
  if (!hw->s16Buff) {
    xmc_sdacDeinit(inst);
    return XMC_ERR_RAM_ALLOC_FAILED;
  }

  uint32_t dmaBuffSize =
      hw->cfg.latencySamples * hw->extraOversample * sizeof(uint32_t) * 2;
  hw->dmaBuff = xmcMalloc(dmaBuffSize, XMC_RAM_CAP_DMA);
  if (!hw->dmaBuff) {
    xmc_sdacDeinit(inst);
    return XMC_ERR_RAM_ALLOC_FAILED;
  }

  hw->nextWriteBank = 0;
  hw->nextReadBank = 0;

  gpio_set_function(inst->pin, GPIO_FUNC_PWM);
  gpio_set_dir(inst->pin, GPIO_OUT);

  hw->pio = pio0;
  hw->pioOffset = pio_add_program(hw->pio, &pioProgram);
  hw->pioSm = pio_claim_unused_sm(hw->pio, true);

  pio_sm_config pioCfg = pio_get_default_sm_config();
  sm_config_set_wrap(&pioCfg, hw->pioOffset + 0, hw->pioOffset + 0);
  sm_config_set_out_pins(&pioCfg, pin, 1);
  sm_config_set_fifo_join(&pioCfg, PIO_FIFO_JOIN_TX);
  sm_config_set_out_shift(&pioCfg, true, true, 32);
  sm_config_set_clkdiv(&pioCfg, (float)sysClkFreq / pdmClkFreq);
  pio_sm_init(hw->pio, hw->pioSm, hw->pioOffset, &pioCfg);

  pio_gpio_init(hw->pio, pin);
  pio_sm_set_consecutive_pindirs(hw->pio, hw->pioSm, pin, 1, true);
  pio_sm_set_enabled(hw->pio, hw->pioSm, true);

  hw->dmaCh = dma_claim_unused_channel(true);
  if (hw->dmaCh < 0) {
    xmc_sdacDeinit(inst);
    return XMC_ERR_DMA_INIT_FAILED;
  }
  dma_channel_set_irq0_enabled(hw->dmaCh, true);
  xmc_dmaRegisterIrqHandler(hw->dmaCh, dmaHandlerFast, dmaHandlerSlow, inst);
  irq_set_exclusive_handler(DMA_IRQ_0, xmc_dmaIrqHandler);
  // irq_add_shared_handler(DMA_IRQ_0, xmc_dmaIrqHandler,
  // PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
  irq_set_enabled(DMA_IRQ_0, true);

  dma_channel_config dma_cfg = dma_channel_get_default_config(hw->dmaCh);
  channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
  channel_config_set_read_increment(&dma_cfg, true);
  channel_config_set_write_increment(&dma_cfg, false);
  channel_config_set_dreq(&dma_cfg, pio_get_dreq(hw->pio, hw->pioSm, true));
  dma_channel_configure(hw->dmaCh, &dma_cfg, &hw->pio->txf[hw->pioSm],
                        hw->dmaBuff,
                        hw->cfg.latencySamples * hw->extraOversample, false);

  fillBuffer(inst);
  fillBuffer(inst);
  startNextDma(inst);

  return XMC_OK;
}

XmcStatus xmc_sdacDeinit(SdacInst *inst) {
  SdacHw *hw = (SdacHw *)inst->hw;
  if (hw) {
    if (hw->dmaCh >= 0) {
      dma_channel_wait_for_finish_blocking(hw->dmaCh);
      dma_channel_unclaim(hw->dmaCh);
      dma_channel_set_irq0_enabled(hw->dmaCh, false);
      xmc_dmaUnregisterIrqHandler(hw->dmaCh);
      irq_set_enabled(DMA_IRQ_0, false);
      hw->dmaCh = -1;
    }
    if (hw->srcFmtBuff) {
      free(hw->srcFmtBuff);
      hw->srcFmtBuff = NULL;
    }
    if (hw->s16Buff) {
      free(hw->s16Buff);
      hw->s16Buff = NULL;
    }
    if (hw->dmaBuff) {
      xmcFree(hw->dmaBuff);
      hw->dmaBuff = NULL;
    }
    pio_sm_set_enabled(hw->pio, hw->pioSm, false);
    pio_sm_unclaim(hw->pio, hw->pioSm);
    pio_remove_program(hw->pio, &pioProgram, hw->pioOffset);
    free(hw);
    inst->hw = NULL;
  }

  // drain charge from pin
  gpio_init(inst->pin);
  gpio_set_dir(inst->pin, GPIO_OUT);
  gpio_put(inst->pin, 0);

  // disable pin
  gpio_deinit(inst->pin);
  return XMC_OK;
}

XmcStatus xmc_sdac_set_source(SdacInst *inst, xmc_audio_source_port_t *src) {
  inst->source = *src;
  return XMC_OK;
}

XmcStatus xmc_sdac_service(SdacInst *inst) {
  SdacHw *hw = (SdacHw *)inst->hw;
  if (hw->nextReadBank == hw->nextWriteBank) {
    fillBuffer(inst);
  }
  return XMC_OK;
}

static void dmaHandlerFast(void *context) {
  SdacInst *inst = (SdacInst *)context;
  startNextDma(inst);
}

static void dmaHandlerSlow(void *context) {
  SdacInst *inst = (SdacInst *)context;
  fillBuffer(inst);
}

static void fillBuffer(SdacInst *inst) {
  SdacHw *hw = (SdacHw *)inst->hw;

  uint32_t dstSamples = hw->cfg.latencySamples * hw->extraOversample;
  uint32_t *dst = hw->dmaBuff + (hw->nextWriteBank * dstSamples);

  if (inst->source.requestData) {
    uint32_t buff_size_bytes =
        hw->cfg.latencySamples *
        xmc_audio_get_bytes_per_sample(hw->cfg.format.sample_format);
    switch (hw->cfg.format.sample_format) {
      default:
      case XMC_SAMPLE_LINEAR_PCM_S16_MONO:
        memset(hw->s16Buff, 0x00, buff_size_bytes);
        inst->source.requestData(hw->s16Buff, hw->cfg.latencySamples,
                                 inst->source.context);
        break;
      case XMC_SAMPLE_LINEAR_PCM_U8_MONO:
        memset(hw->srcFmtBuff, 0x80, buff_size_bytes);
        inst->source.requestData(hw->srcFmtBuff, hw->cfg.latencySamples,
                                 inst->source.context);
        for (int i = 0; i < hw->cfg.latencySamples; i++) {
          hw->s16Buff[i] = ((int16_t)hw->srcFmtBuff[i] - 128) << 8;
        }
        break;
    }

    int16_t *src = (int16_t *)hw->s16Buff;
    int32_t lastInput = hw->pdmLastInput;
    int32_t lastQt = hw->pdmLastQt;
    for (int isrc = 0; isrc < hw->cfg.latencySamples; isrc++) {
      int32_t newInput = src[isrc] * 0x100 + 0x1000;

      for (int ieos = 0; ieos < hw->extraOversample; ieos++) {
        uint32_t pdmOutput = 0;
        // updateLfsr(&(hw->pdmLfsr));
        // updateLfsr(&(hw->pdmLfsr));
        // updateLfsr(&(hw->pdmLfsr));
        // uint32_t dither = hw->pdmLfsr;
        updateLfsr(&(hw->pdmLfsr));
        int32_t dither = (int32_t)(hw->pdmLfsr & 0x4) - 0x2;
        // uint32_t noise = hw->pdmLfsr;
        //  int32_t dither = (hw->pdmLfsr & 0xF) - 0x8;
        //  newInput += (noise & 0x2) - 0x1;
#if 0
            // todo: use interpolator
            int32_t over_sample = (newInput * j + lastInput * (32 - j)) / 32;
#else
        int32_t over_sample = newInput;
#endif
        for (int ibit = 0; ibit < 32; ibit++) {
          // over_sample += ((dither & 1) * 2 - 1) * 0x800;
          // dither >>= 1;
          // int32_t dither = ((noise & 1) * 2 - 1) * 0xFF;
          // noise >>= 1;

          // over_sample += dither;

          // const int dither_period = 0x80;
          // const int dither_amp = 4;
          // hw->ditherPhase = (hw->ditherPhase + 1) & (dither_period - 1);
          // int32_t dither = hw->ditherPhase;
          // if (dither >= dither_period / 2) {
          //   dither = dither_period - dither;
          // }
          // dither -= dither_period / 4;
          // dither *= dither_amp;

          hw->pdmWork0 += over_sample - lastQt;
          hw->pdmWork1 += hw->pdmWork0 - lastQt;

          // hw->pdmWork1 += dither;
          // updateLfsr(&(hw->pdmLfsr));
          // int32_t dither = (int32_t)(hw->pdmLfsr & 0xFFFF) - 0x8000;

          // hw->pdmWork2 += hw->pdmWork1 - lastQt;
          // hw->pdmWork3 += hw->pdmWork2 - lastQt;
          pdmOutput >>= 1;
          if (hw->pdmWork1 >= 0) {
            pdmOutput |= 0x80000000;
            lastQt = 0xFEDCBA;
          } else {
            lastQt = -0xFEDCBA;
          }
        }
        lastInput = newInput;
        dst[isrc * hw->extraOversample + ieos] = pdmOutput;
        // hw->pdmWork0 -= hw->pdmWork0 / 256;
        // hw->pdmWork1 -= hw->pdmWork1 / 256;
      }
      hw->pdmLastInput = lastInput;
      hw->pdmLastQt = lastQt;
    }
  } else {
    uint32_t sample = 0x55555555;
    for (int i = 0; i < dstSamples; i++) {
      dst[i] = sample;
    }
  }
  hw->nextWriteBank = (hw->nextWriteBank + 1) % 2;
}

static void startNextDma(SdacInst *inst) {
  SdacHw *hw = (SdacHw *)inst->hw;

  uint32_t dstSamples = hw->cfg.latencySamples * hw->extraOversample;
  dma_channel_set_read_addr(
      hw->dmaCh, hw->dmaBuff + (hw->nextReadBank * dstSamples), true);
  hw->nextReadBank = (hw->nextReadBank + 1) % 2;
}
