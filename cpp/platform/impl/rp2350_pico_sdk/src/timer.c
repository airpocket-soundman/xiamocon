#include "xmc/hw/timer.h"

#include <pico/stdlib.h>
#include <stdlib.h>

uint64_t xmc_get_time_ms() { return to_ms_since_boot(get_absolute_time()); }
uint64_t xmc_get_time_us() { return to_us_since_boot(get_absolute_time()); }
void xmc_sleep_ms(uint32_t ms) { sleep_ms(ms); }
void xmc_sleep_us(uint32_t us) { sleep_us(us); }

typedef struct {
  xmc_timer_tick_t cb;
  void *context;
} repeating_timer_hw_t;

static bool repeating_timer_callback(repeating_timer_t *rt);

xmc_status_t xmc_timer_add_repeating_ms(xmc_repeating_timer_t *timer,
                                        uint32_t interval_ms,
                                        xmc_timer_tick_t cb, void *context) {
  repeating_timer_hw_t *hw = malloc(sizeof(repeating_timer_hw_t));
  hw->cb = cb;
  hw->context = context;
  repeating_timer_t *handle = malloc(sizeof(repeating_timer_t));
  timer->handle = handle;
  timer->tick = cb;
  timer->context = hw;
  if (!add_repeating_timer_ms(interval_ms, repeating_timer_callback, hw,
                              handle)) {
    free(handle);
    return XMC_ERR_TIMER_REPEATING_TIMER_INIT_FAILED;
  }
  return XMC_OK;
}

void xmc_timer_cancel_repeating(xmc_repeating_timer_t *timer) {
  if (timer && timer->handle) {
    repeating_timer_t *handle = (repeating_timer_t *)timer->handle;
    cancel_repeating_timer(handle);
    free(handle);
    timer->handle = NULL;
  }
}

static bool repeating_timer_callback(repeating_timer_t *rt) {
  repeating_timer_hw_t *hw = (repeating_timer_hw_t *)rt->user_data;
  return hw->cb(hw->context);
}
