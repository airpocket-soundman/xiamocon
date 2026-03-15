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
  xmc_sdac_config_t cfg;
  xmc_audio_source_port_t *source;
  PIO pio;
  int pio_sm;
  int pio_offset;
  int dma_ch;
  int next_write_bank;
  int next_read_bank;
  uint8_t *src_fmt_buff;
  int16_t *s16_buff;
  uint32_t *dma_buff;
  // uint16_t dither_phase;
  uint32_t extra_oversample;
  int32_t pdm_last_input;
  int32_t pdm_last_qt;
  int32_t pdm_work0;
  int32_t pdm_work1;
  // int32_t pdm_work2;
  // int32_t pdm_work3;
  uint32_t pdm_lfsr;
} xmc_sdac_hw_t;

static const uint16_t pio_instructions[] = {
    0x6001,
};

static const struct pio_program pio_program = {
    .instructions = pio_instructions,
    .length = 1,
    .origin = -1,
};

static void start_next_dma(xmc_sdac_inst_t *inst);
static void fill_buffer(xmc_sdac_inst_t *inst);
static void dma_handler_fast(void *context);
static void dma_handler_slow(void *context);

static inline void update_lfsr(uint32_t *lfsr) {
  uint32_t bit =
      ((*lfsr >> 0) ^ (*lfsr >> 10) ^ (*lfsr >> 30) ^ (*lfsr >> 31)) & 1;
  *lfsr = (*lfsr >> 1) | (bit << 31);
}

#define SUPPORTED_FORMATS \
  (XMC_SAMPLE_LINEAR_PCM_U8_MONO | XMC_SAMPLE_LINEAR_PCM_S16_MONO)

xmc_audio_sample_format_t xmc_sdac_get_supported_formats(void) {
  return SUPPORTED_FORMATS;
}

xmc_status_t xmc_sdac_init(xmc_sdac_inst_t *inst, int pin,
                           const xmc_sdac_config_t *cfg,
                           float *actual_rate_hz) {
  // todo: calculate actual_rate_hz based on cfg->format.sample_rate_hz and
  // hardware capabilities
  *actual_rate_hz = cfg->format.sample_rate_hz;

  xmc_sdac_hw_t *hw = malloc(sizeof(xmc_sdac_hw_t));
  if (!hw) {
    return XMC_ERR_RAM_ALLOC_FAILED;
  }
  inst->hw = hw;
  inst->pin = pin;

  hw->cfg = *cfg;
  hw->pdm_work0 = 0;
  hw->pdm_work1 = 0;
  // hw->pdm_work2 = 0;
  // hw->pdm_work3 = 0;
  hw->pdm_last_input = 0;
  hw->pdm_lfsr = 0xFFFFFFFF;

  uint32_t sys_clk_freq = clock_get_hz(clk_sys);
  uint32_t pdm_clk_freq = hw->cfg.format.sample_rate_hz * 32;
  hw->extra_oversample =
      (PDM_MIN_PDM_FREQ_HZ + pdm_clk_freq - 1) / pdm_clk_freq;
  if (hw->extra_oversample < 1) hw->extra_oversample = 1;
  pdm_clk_freq *= hw->extra_oversample;

  if ((cfg->format.sample_format & SUPPORTED_FORMATS) == 0) {
    XMC_ERR_RET(XMC_ERR_SPEAKER_UNSUPPORTED_FORMAT);
  }

  if (cfg->format.sample_format != XMC_SAMPLE_LINEAR_PCM_S16_MONO) {
    int bytes_per_sample =
        xmc_audio_get_bytes_per_sample(hw->cfg.format.sample_format);
    hw->src_fmt_buff = malloc(hw->cfg.latency_samples * bytes_per_sample);
    if (!hw->src_fmt_buff) {
      xmc_sdac_deinit(inst);
      return XMC_ERR_RAM_ALLOC_FAILED;
    }
  } else {
    hw->src_fmt_buff = NULL;
  }

  hw->s16_buff = malloc(hw->cfg.latency_samples * sizeof(int16_t));
  if (!hw->s16_buff) {
    xmc_sdac_deinit(inst);
    return XMC_ERR_RAM_ALLOC_FAILED;
  }

  uint32_t dma_buff_size =
      hw->cfg.latency_samples * hw->extra_oversample * sizeof(uint32_t) * 2;
  hw->dma_buff = xmc_malloc(dma_buff_size, XMC_RAM_CAP_DMA);
  if (!hw->dma_buff) {
    xmc_sdac_deinit(inst);
    return XMC_ERR_RAM_ALLOC_FAILED;
  }

  hw->next_write_bank = 0;
  hw->next_read_bank = 0;

  gpio_set_function(inst->pin, GPIO_FUNC_PWM);
  gpio_set_dir(inst->pin, GPIO_OUT);

  hw->pio = pio0;
  hw->pio_offset = pio_add_program(hw->pio, &pio_program);
  hw->pio_sm = pio_claim_unused_sm(hw->pio, true);

  pio_sm_config pio_cfg = pio_get_default_sm_config();
  sm_config_set_wrap(&pio_cfg, hw->pio_offset + 0, hw->pio_offset + 0);
  sm_config_set_out_pins(&pio_cfg, pin, 1);
  sm_config_set_fifo_join(&pio_cfg, PIO_FIFO_JOIN_TX);
  sm_config_set_out_shift(&pio_cfg, true, true, 32);
  sm_config_set_clkdiv(&pio_cfg, (float)sys_clk_freq / pdm_clk_freq);
  pio_sm_init(hw->pio, hw->pio_sm, hw->pio_offset, &pio_cfg);

  pio_gpio_init(hw->pio, pin);
  pio_sm_set_consecutive_pindirs(hw->pio, hw->pio_sm, pin, 1, true);
  pio_sm_set_enabled(hw->pio, hw->pio_sm, true);

  hw->dma_ch = dma_claim_unused_channel(true);
  if (hw->dma_ch < 0) {
    xmc_sdac_deinit(inst);
    return XMC_ERR_DMA_INIT_FAILED;
  }
  dma_channel_set_irq0_enabled(hw->dma_ch, true);
  xmc_dma_register_irq_handler(hw->dma_ch, dma_handler_fast, dma_handler_slow,
                               inst);
  irq_set_exclusive_handler(DMA_IRQ_0, xmc_dma_irq_handler);
  // irq_add_shared_handler(DMA_IRQ_0, xmc_dma_irq_handler,
  // PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
  irq_set_enabled(DMA_IRQ_0, true);

  dma_channel_config dma_cfg = dma_channel_get_default_config(hw->dma_ch);
  channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
  channel_config_set_read_increment(&dma_cfg, true);
  channel_config_set_write_increment(&dma_cfg, false);
  channel_config_set_dreq(&dma_cfg, pio_get_dreq(hw->pio, hw->pio_sm, true));
  dma_channel_configure(hw->dma_ch, &dma_cfg, &hw->pio->txf[hw->pio_sm],
                        hw->dma_buff,
                        hw->cfg.latency_samples * hw->extra_oversample, false);

  fill_buffer(inst);
  fill_buffer(inst);
  start_next_dma(inst);

  return XMC_OK;
}

xmc_status_t xmc_sdac_deinit(xmc_sdac_inst_t *inst) {
  xmc_sdac_hw_t *hw = (xmc_sdac_hw_t *)inst->hw;
  if (hw) {
    if (hw->dma_ch >= 0) {
      dma_channel_wait_for_finish_blocking(hw->dma_ch);
      dma_channel_unclaim(hw->dma_ch);
      dma_channel_set_irq0_enabled(hw->dma_ch, false);
      xmc_dma_unregister_irq_handler(hw->dma_ch);
      irq_set_enabled(DMA_IRQ_0, false);
      hw->dma_ch = -1;
    }
    if (hw->src_fmt_buff) {
      free(hw->src_fmt_buff);
      hw->src_fmt_buff = NULL;
    }
    if (hw->s16_buff) {
      free(hw->s16_buff);
      hw->s16_buff = NULL;
    }
    if (hw->dma_buff) {
      xmc_free(hw->dma_buff);
      hw->dma_buff = NULL;
    }
    pio_sm_set_enabled(hw->pio, hw->pio_sm, false);
    pio_sm_unclaim(hw->pio, hw->pio_sm);
    pio_remove_program(hw->pio, &pio_program, hw->pio_offset);
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

xmc_status_t xmc_sdac_set_source(xmc_sdac_inst_t *inst,
                                 xmc_audio_source_port_t *src) {
  inst->source = *src;
  return XMC_OK;
}

xmc_status_t xmc_sdac_service(xmc_sdac_inst_t *inst) {
  xmc_sdac_hw_t *hw = (xmc_sdac_hw_t *)inst->hw;
  if (hw->next_read_bank == hw->next_write_bank) {
    fill_buffer(inst);
  }
  return XMC_OK;
}

static void dma_handler_fast(void *context) {
  xmc_sdac_inst_t *inst = (xmc_sdac_inst_t *)context;
  start_next_dma(inst);
}

static void dma_handler_slow(void *context) {
  xmc_sdac_inst_t *inst = (xmc_sdac_inst_t *)context;
  fill_buffer(inst);
}

static void fill_buffer(xmc_sdac_inst_t *inst) {
  xmc_sdac_hw_t *hw = (xmc_sdac_hw_t *)inst->hw;

  uint32_t dst_samples = hw->cfg.latency_samples * hw->extra_oversample;
  uint32_t *dst = hw->dma_buff + (hw->next_write_bank * dst_samples);

  if (inst->source.request_data) {
    uint32_t buff_size_bytes =
        hw->cfg.latency_samples *
        xmc_audio_get_bytes_per_sample(hw->cfg.format.sample_format);
    switch (hw->cfg.format.sample_format) {
      default:
      case XMC_SAMPLE_LINEAR_PCM_S16_MONO:
        memset(hw->s16_buff, 0x00, buff_size_bytes);
        inst->source.request_data(hw->s16_buff, hw->cfg.latency_samples,
                                  inst->source.context);
        break;
      case XMC_SAMPLE_LINEAR_PCM_U8_MONO:
        memset(hw->src_fmt_buff, 0x80, buff_size_bytes);
        inst->source.request_data(hw->src_fmt_buff, hw->cfg.latency_samples,
                                  inst->source.context);
        for (int i = 0; i < hw->cfg.latency_samples; i++) {
          hw->s16_buff[i] = ((int16_t)hw->src_fmt_buff[i] - 128) << 8;
        }
        break;
    }

    int16_t *src = (int16_t *)hw->s16_buff;
    int32_t last_input = hw->pdm_last_input;
    int32_t last_qt = hw->pdm_last_qt;
    for (int isrc = 0; isrc < hw->cfg.latency_samples; isrc++) {
      int32_t new_input = src[isrc] * 0x100 + 0x1000;

      for (int ieos = 0; ieos < hw->extra_oversample; ieos++) {
        uint32_t pdm_output = 0;
        // update_lfsr(&(hw->pdm_lfsr));
        // update_lfsr(&(hw->pdm_lfsr));
        // update_lfsr(&(hw->pdm_lfsr));
        // uint32_t dither = hw->pdm_lfsr;
        update_lfsr(&(hw->pdm_lfsr));
        int32_t dither = (int32_t)(hw->pdm_lfsr & 0x4) - 0x2;
        // uint32_t noise = hw->pdm_lfsr;
        //  int32_t dither = (hw->pdm_lfsr & 0xF) - 0x8;
        //  new_input += (noise & 0x2) - 0x1;
#if 0
            // todo: use interpolator
            int32_t over_sample = (new_input * j + last_input * (32 - j)) / 32;
#else
        int32_t over_sample = new_input;
#endif
        for (int ibit = 0; ibit < 32; ibit++) {
          // over_sample += ((dither & 1) * 2 - 1) * 0x800;
          // dither >>= 1;
          // int32_t dither = ((noise & 1) * 2 - 1) * 0xFF;
          // noise >>= 1;

          // over_sample += dither;

          // const int dither_period = 0x80;
          // const int dither_amp = 4;
          // hw->dither_phase = (hw->dither_phase + 1) & (dither_period - 1);
          // int32_t dither = hw->dither_phase;
          // if (dither >= dither_period / 2) {
          //   dither = dither_period - dither;
          // }
          // dither -= dither_period / 4;
          // dither *= dither_amp;

          hw->pdm_work0 += over_sample - last_qt;
          hw->pdm_work1 += hw->pdm_work0 - last_qt;

          // hw->pdm_work1 += dither;
          // update_lfsr(&(hw->pdm_lfsr));
          // int32_t dither = (int32_t)(hw->pdm_lfsr & 0xFFFF) - 0x8000;

          // hw->pdm_work2 += hw->pdm_work1 - last_qt;
          // hw->pdm_work3 += hw->pdm_work2 - last_qt;
          pdm_output >>= 1;
          if (hw->pdm_work1 >= 0) {
            pdm_output |= 0x80000000;
            last_qt = 0xFEDCBA;
          } else {
            last_qt = -0xFEDCBA;
          }
        }
        last_input = new_input;
        dst[isrc * hw->extra_oversample + ieos] = pdm_output;
        // hw->pdm_work0 -= hw->pdm_work0 / 256;
        // hw->pdm_work1 -= hw->pdm_work1 / 256;
      }
      hw->pdm_last_input = last_input;
      hw->pdm_last_qt = last_qt;
    }
  } else {
    uint32_t sample = 0x55555555;
    for (int i = 0; i < dst_samples; i++) {
      dst[i] = sample;
    }
  }
  hw->next_write_bank = (hw->next_write_bank + 1) % 2;
}

static void start_next_dma(xmc_sdac_inst_t *inst) {
  xmc_sdac_hw_t *hw = (xmc_sdac_hw_t *)inst->hw;

  uint32_t dst_samples = hw->cfg.latency_samples * hw->extra_oversample;
  dma_channel_set_read_addr(
      hw->dma_ch, hw->dma_buff + (hw->next_read_bank * dst_samples), true);
  hw->next_read_bank = (hw->next_read_bank + 1) % 2;
}
