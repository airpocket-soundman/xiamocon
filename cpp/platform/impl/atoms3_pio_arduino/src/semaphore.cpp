#include "xmc/hw/semaphore.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdlib.h>

namespace xmc {

Semaphore::Semaphore() {
  SemaphoreHandle_t sem = xSemaphoreCreateBinary();
  if (!sem) return;
  handle = sem;
  xSemaphoreGive(sem);
}

Semaphore::~Semaphore() {
  if (!handle) return;
  vSemaphoreDelete((SemaphoreHandle_t)handle);
  handle = nullptr;
}

void Semaphore::take() {
  if (!handle) return;
  xSemaphoreTake((SemaphoreHandle_t)handle, portMAX_DELAY);
}

bool Semaphore::tryTake() {
  if (!handle) return false;
  return xSemaphoreTake((SemaphoreHandle_t)handle, 0) == pdTRUE;
}

void Semaphore::give() {
  if (!handle) return;
  xSemaphoreGive((SemaphoreHandle_t)handle);
}

}  // namespace xmc
