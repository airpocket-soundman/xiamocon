#include "xmc/hw/pwm.h"

#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/pwm.h>
#include <pico/stdlib.h>
#include <stdlib.h>

typedef struct {
  uint slice_num;
  uint channel;
  bool dma_inited;
  uint dma_ch;
} xmc_pwm_hw_t;

bool xmc_pwm_supports_dma() { return true; }

xmc_status_t xmc_pwm_init(xmc_pwm_inst_t *inst, const xmc_pwm_config_t *cfg, float *actual_freq_hz) {
  uint slice_num = pwm_gpio_to_slice_num(cfg->pin);
  pwm_config p_config = pwm_get_default_config();
  pwm_config_set_clkdiv(
      &p_config, (float)clock_get_hz(clk_sys) / cfg->freq_hz / cfg->period);
  pwm_init(slice_num, &p_config, true);
  gpio_set_function(cfg->pin, GPIO_FUNC_PWM);

  if (actual_freq_hz) {
    *actual_freq_hz = (float)clock_get_hz(clk_sys) / (pwm_get_wrap(slice_num) + 1) / pwm_get_clkdiv(slice_num);
  }

  xmc_pwm_hw_t *hw = malloc(sizeof(xmc_pwm_hw_t));
  if (!hw) {
    return XMC_ERR_RAM_ALLOC_FAILED;
  }
  inst->handle = hw;
  hw->slice_num = slice_num;
  hw->channel = pwm_gpio_to_channel(cfg->pin);
  hw->dma_inited = false;
  return XMC_OK;
}

xmc_status_t xmc_pwm_deinit(xmc_pwm_inst_t *inst) {
  xmc_pwm_hw_t *hw = inst->handle;
  pwm_set_enabled(hw->slice_num, false);
  if (hw->dma_inited) {
    dma_channel_wait_for_finish_blocking(hw->dma_ch);
    dma_channel_unclaim(hw->dma_ch);
  }
  free(hw);
  inst->handle = NULL;
  return XMC_OK;
}

xmc_status_t xmc_pwm_set_duty_cycle(xmc_pwm_inst_t *inst, uint32_t cycle) {
  xmc_pwm_hw_t *hw = inst->handle;
#if 0
  if (cycle > pwm_hw->slice[slice_num].top) {
    return XMC_ERR_PWM_INVALID_DUTY_CYCLE;
  }
#endif
  pwm_set_chan_level(hw->slice_num, PWM_CHAN_A, cycle);
  return XMC_OK;
}

xmc_status_t xmc_pwm_dma_write_start(xmc_pwm_inst_t *inst,
                                     const xmc_dma_config_t *cfg) {
  xmc_pwm_hw_t *hw = inst->handle;
  if (!hw->dma_inited) {
    hw->dma_ch = dma_claim_unused_channel(true);
    hw->dma_inited = true;
  }
  dma_channel_config dma_cfg = dma_channel_get_default_config(hw->dma_ch);
  int tx_size;
  switch (cfg->element_size) {
    case 1: tx_size = DMA_SIZE_8; break;
    case 2: tx_size = DMA_SIZE_16; break;
    case 4: tx_size = DMA_SIZE_32; break;
    default: return XMC_ERR_DMA_BAD_ELEMENT_SIZE;
  }
  channel_config_set_transfer_data_size(&dma_cfg, tx_size);
  channel_config_set_read_increment(&dma_cfg, true);
  channel_config_set_write_increment(&dma_cfg, false);
  channel_config_set_dreq(&dma_cfg, DREQ_PWM_WRAP0 + hw->slice_num);
  channel_config_set_chain_to(&dma_cfg, hw->dma_ch);
  channel_config_set_enable(&dma_cfg, true);
  dma_channel_configure(hw->dma_ch, &dma_cfg, &pwm_hw->slice[hw->slice_num].cc,
                        cfg->ptr, cfg->length, false);
  return XMC_OK;
}

xmc_status_t xmc_pwm_dma_complete(xmc_pwm_inst_t *inst) {
  xmc_pwm_hw_t *hw = inst->handle;
  if (hw->dma_inited) {
    dma_channel_wait_for_finish_blocking(hw->dma_ch);
  }
  return XMC_OK;
}

bool xmc_pwm_dma_is_busy(xmc_pwm_inst_t *inst) {
  xmc_pwm_hw_t *hw = inst->handle;
  if (hw->dma_inited) {
    return dma_channel_is_busy(hw->dma_ch);
  }
  return false;
}