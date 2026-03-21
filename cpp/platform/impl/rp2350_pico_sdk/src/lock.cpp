#include "xmc/hw/lock.h"

#include <pico/sem.h>
#include <pico/stdlib.h>
#include <stdlib.h>

static const int NUM_SPINLOCKS = 16;
static uint32_t usedSpinlocks = 0;

typedef struct {
  spin_lock_t *hwLock;
  uint32_t irqs;
  int id;
} SpinLockHw;

#if defined(__cplusplus)
extern "C" {
#endif

XmcStatus xmc_spinlockInit(SpinLock *lock) {
  int id = __builtin_ffs(~usedSpinlocks) - 1;
  if (id < 0 || NUM_SPINLOCKS <= id) {
    XMC_ERR_RET(XMC_ERR_SPINLOCK_INIT_FAILED);
  }

  SpinLockHw *hw = (SpinLockHw *)malloc(sizeof(SpinLockHw));
  hw->hwLock = spin_lock_init(id);
  hw->id = id;
  lock->handle = hw;
  spin_lock_claim(id);
  usedSpinlocks |= (1U << id);
  return XMC_OK;
}

void xmc_spinlockDeinit(SpinLock *lock) {
  SpinLockHw *hw = (SpinLockHw *)(lock->handle);
  if (!hw) return;
  spin_lock_unclaim(hw->id);
  usedSpinlocks &= ~(1U << hw->id);
  free(hw);
  lock->handle = NULL;
}

void xmc_spinlockGet(SpinLock *lock) {
  SpinLockHw *hw = (SpinLockHw *)(lock->handle);
  if (!hw) return;
  hw->irqs = spin_lock_blocking(hw->hwLock);
}

void xmc_spinlockRelease(SpinLock *lock) {
  SpinLockHw *hw = (SpinLockHw *)(lock->handle);
  if (!hw) return;
  spin_unlock(hw->hwLock, hw->irqs);
}

XmcStatus xmc_semaphoreInit(Semaphore *sem) {
  semaphore_t *semHandle = (semaphore_t *)malloc(sizeof(semaphore_t));
  if (!semHandle) {
    return XMC_ERR_SEMAPHORE_INIT_FAILED;
  }
  sem->handle = semHandle;
  sem_init(semHandle, 1, 1);
  return XMC_OK;
}

void xmc_semaphoreDeinit(Semaphore *sem) {
  free(sem->handle);
  sem->handle = NULL;
}

void xmc_semaphoreTake(Semaphore *sem) {
  sem_acquire_blocking((semaphore_t *)(sem->handle));
}

bool xmc_semaphoreTryTake(Semaphore *sem) {
  return sem_try_acquire((semaphore_t *)(sem->handle));
}

void xmc_semaphoreGive(Semaphore *sem) {
  sem_release((semaphore_t *)(sem->handle));
}

#if defined(__cplusplus)
}
#endif
