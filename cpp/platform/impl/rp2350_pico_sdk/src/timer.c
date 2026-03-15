#include "xmc/hw/timer.h"

#include <pico/stdlib.h>

uint64_t xmc_get_time_ms() { return to_ms_since_boot(get_absolute_time()); }
uint64_t xmc_get_time_us() { return to_us_since_boot(get_absolute_time()); }
void xmc_sleep_ms(uint32_t ms) { sleep_ms(ms); }
void xmc_sleep_us(uint32_t us) { sleep_us(us); }

xmc_status_t xmc_timer_add_repeating_ms(xmc_repeating_timer_t *timer,
                                        uint32_t interval_ms,
                                        xmc_timer_tick_t cb, void *context) {
  repeating_timer_t *handle = malloc(sizeof(repeating_timer_t));
  timer->handle = handle;
  timer->tick = cb;
  timer->context = context;
  if (!add_repeating_timer_ms(interval_ms, cb, context, handle)) {
    free(handle);
    return XMC_ERR_TIMER_REPEATING_TIMER_INIT_FAILED;
  }
  return XMC_OK;
}

void xmc_timer_cancel_repeating(xmc_repeating_timer_t *timer) {
  if (timer && timer->handle) {
    repeating_timer_t *handle = (repeating_timer_t *)timer->handle;
    cancel_repeating_timer(handle->alarm_id);
    free(handle);
    timer->handle = NULL;
  }
}
