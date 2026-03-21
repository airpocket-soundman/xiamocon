#include "xmc/hw/pwm.h"

#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/pwm.h>
#include <pico/stdlib.h>
#include <stdlib.h>

typedef struct {
  uint sliceNum;
  uint channel;
} PwmHw;

XmcStatus xmc_pwmInit(xmc_pwm_inst_t *inst, const xmc_pwm_config_t *cfg,
                          float *actualFreqHz) {
  uint32_t periClkFreq = clock_get_hz(clk_peri);
  uint sliceNum = pwm_gpio_to_slice_num(cfg->pin);
  pwm_config pwmCfg = pwm_get_default_config();
  pwm_config_set_clkdiv(&pwmCfg,
                        (float)periClkFreq / cfg->freq_hz / cfg->period);
  pwm_init(sliceNum, &pwmCfg, true);
  gpio_set_function(cfg->pin, GPIO_FUNC_PWM);

  if (actualFreqHz) {
    pwm_slice_hw_t *hw = &(pwm_hw->slice[sliceNum]);
    *actualFreqHz =
        (float)periClkFreq / (hw->top + 1) / ((float)hw->div / 16);
  }

  PwmHw *hw = malloc(sizeof(PwmHw));
  if (!hw) {
    return XMC_ERR_RAM_ALLOC_FAILED;
  }
  inst->handle = hw;
  hw->sliceNum = sliceNum;
  hw->channel = pwm_gpio_to_channel(cfg->pin);
  return XMC_OK;
}

XmcStatus xmc_pwmDeinit(xmc_pwm_inst_t *inst) {
  PwmHw *hw = inst->handle;
  pwm_set_enabled(hw->sliceNum, false);
  free(hw);
  inst->handle = NULL;
  return XMC_OK;
}

XmcStatus xmc_pwmSetDutyCycle(xmc_pwm_inst_t *inst, uint32_t cycle) {
  PwmHw *hw = inst->handle;
#if 0
  if (cycle > pwm_hw->slice[sliceNum].top) {
    return XMC_ERR_PWM_INVALID_DUTY_CYCLE;
  }
#endif
  pwm_set_chan_level(hw->sliceNum, PWM_CHAN_A, cycle);
  return XMC_OK;
}
