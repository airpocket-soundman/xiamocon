#include "xmc/hw/timer.hpp"

#include <Arduino.h>
#include <esp_timer.h>
#include <stdlib.h>

namespace xmc {

uint64_t getTimeMs() { return (uint64_t)millis(); }
uint64_t getTimeUs() { return (uint64_t)micros(); }
void sleepMs(uint32_t ms) { delay(ms); }
void sleepUs(uint32_t us) { delayMicroseconds(us); }

struct RepeatingTimerHw {
  esp_timer_handle_t timer;
  TimerCallback cb;
  void *context;
};

static void esp_timer_cb(void *arg) {
  RepeatingTimerHw *hw = (RepeatingTimerHw *)arg;
  if (!hw->cb(hw->context)) {
    esp_timer_stop(hw->timer);
  }
}

RepeatingTimer::RepeatingTimer() {
  RepeatingTimerHw *hw = (RepeatingTimerHw *)malloc(sizeof(RepeatingTimerHw));
  if (!hw) return;
  hw->timer = nullptr;
  hw->cb = nullptr;
  hw->context = nullptr;
  handle = hw;
}

RepeatingTimer::~RepeatingTimer() {
  if (!handle) return;
  RepeatingTimerHw *hw = (RepeatingTimerHw *)handle;
  if (hw->timer) {
    esp_timer_stop(hw->timer);
    esp_timer_delete(hw->timer);
  }
  free(hw);
  handle = nullptr;
}

XmcStatus RepeatingTimer::startMs(uint32_t intervalMs, TimerCallback cb,
                                   void *context) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  RepeatingTimerHw *hw = (RepeatingTimerHw *)handle;
  hw->cb = cb;
  hw->context = context;

  esp_timer_create_args_t args = {
      .callback = esp_timer_cb,
      .arg = hw,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "xmc_timer",
      .skip_unhandled_events = false,
  };
  if (esp_timer_create(&args, &hw->timer) != ESP_OK) {
    XMC_ERR_RET(XMC_ERR_TIMER_REPEATING_TIMER_INIT_FAILED);
  }
  if (esp_timer_start_periodic(hw->timer,
                                (uint64_t)intervalMs * 1000) != ESP_OK) {
    esp_timer_delete(hw->timer);
    hw->timer = nullptr;
    XMC_ERR_RET(XMC_ERR_TIMER_REPEATING_TIMER_INIT_FAILED);
  }
  return XMC_OK;
}

void RepeatingTimer::cancel() {
  if (!handle) return;
  RepeatingTimerHw *hw = (RepeatingTimerHw *)handle;
  if (hw->timer) esp_timer_stop(hw->timer);
}

}  // namespace xmc
