#include "xmc/hw/timer.h"

#include <pico/stdlib.h>
#include <stdlib.h>

uint64_t xmc_getTimeMs() { return to_ms_since_boot(get_absolute_time()); }
uint64_t xmc_getTimeUs() { return to_us_since_boot(get_absolute_time()); }
void xmc_sleepMs(uint32_t ms) { sleep_ms(ms); }
void xmc_sleepUs(uint32_t us) { sleep_us(us); }

typedef struct {
  TimerTickCb cb;
  void *context;
} RepeatingTimerHw;

static bool repeatingTimerCallback(repeating_timer_t *rt);

XmcStatus xmc_timerAddRepeatingMs(RepeatingTimer *timer,
                                        uint32_t interval_ms,
                                        TimerTickCb cb, void *context) {
  RepeatingTimerHw *hw = malloc(sizeof(RepeatingTimerHw));
  hw->cb = cb;
  hw->context = context;
  repeating_timer_t *handle = malloc(sizeof(repeating_timer_t));
  timer->handle = handle;
  timer->tick = cb;
  timer->context = hw;
  if (!add_repeating_timer_ms(interval_ms, repeatingTimerCallback, hw,
                              handle)) {
    free(handle);
    return XMC_ERR_TIMER_REPEATING_TIMER_INIT_FAILED;
  }
  return XMC_OK;
}

void xmc_timerCancelRepeating(RepeatingTimer *timer) {
  if (timer && timer->handle) {
    repeating_timer_t *handle = (repeating_timer_t *)timer->handle;
    cancel_repeating_timer(handle);
    free(handle);
    timer->handle = NULL;
  }
}

static bool repeatingTimerCallback(repeating_timer_t *rt) {
  RepeatingTimerHw *hw = (RepeatingTimerHw *)rt->user_data;
  return hw->cb(hw->context);
}
