#include "xmc/hw/timer.hpp"

#include <pico/stdlib.h>
#include <stdlib.h>

namespace xmc {

struct RepeatingTimerHw {
  repeating_timer_t *timer;
  TimerCallback cb;
  void *context;
};

static bool repeatingTimerCallback(repeating_timer_t *rt);

uint64_t getTimeMs() { return to_ms_since_boot(get_absolute_time()); }
uint64_t getTimeUs() { return to_us_since_boot(get_absolute_time()); }
void sleepMs(uint32_t ms) { sleep_ms(ms); }
void sleepUs(uint32_t us) { sleep_us(us); }

RepeatingTimer::RepeatingTimer() {
  RepeatingTimerHw *hw = (RepeatingTimerHw *)malloc(sizeof(RepeatingTimerHw));
  if (!hw) return;
  handle = hw;
  hw->timer = (repeating_timer_t *)malloc(sizeof(repeating_timer_t));
}

RepeatingTimer::~RepeatingTimer() {
  if (handle) {
    RepeatingTimerHw *hw = (RepeatingTimerHw *)handle;
    if (hw->timer) {
      cancel_repeating_timer(hw->timer);
      free(hw->timer);
    }
    free(handle);
    handle = nullptr;
  }
}

XmcStatus RepeatingTimer ::startMs(uint32_t intervalMs, TimerCallback cb,
                                   void *context) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  RepeatingTimerHw *hw = (RepeatingTimerHw *)handle;
  hw->cb = cb;
  hw->context = context;
  if (!add_repeating_timer_ms(intervalMs, repeatingTimerCallback, hw,
                              hw->timer)) {
    free(handle);
    return XMC_ERR_TIMER_REPEATING_TIMER_INIT_FAILED;
  }
  return XMC_OK;
}

void RepeatingTimer::cancel() {
  if (!handle) return;
  RepeatingTimerHw *hw = (RepeatingTimerHw *)handle;
  if (!hw->timer) return;
  cancel_repeating_timer(hw->timer);
}

static bool repeatingTimerCallback(repeating_timer_t *rt) {
  RepeatingTimerHw *hw = (RepeatingTimerHw *)rt->user_data;
  return hw->cb(hw->context);
}

}  // namespace xmc