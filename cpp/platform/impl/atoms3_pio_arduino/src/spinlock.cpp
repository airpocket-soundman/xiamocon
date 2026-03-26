#include "xmc/hw/spinlock.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdlib.h>

namespace xmc {

SpinLock::SpinLock() {
  SemaphoreHandle_t sem = xSemaphoreCreateMutex();
  if (!sem) return;
  handle = sem;
}

SpinLock::~SpinLock() {
  if (!handle) return;
  vSemaphoreDelete((SemaphoreHandle_t)handle);
  handle = nullptr;
}

void SpinLock::get() {
  if (!handle) return;
  xSemaphoreTake((SemaphoreHandle_t)handle, portMAX_DELAY);
}

void SpinLock::release() {
  if (!handle) return;
  xSemaphoreGive((SemaphoreHandle_t)handle);
}

}  // namespace xmc
