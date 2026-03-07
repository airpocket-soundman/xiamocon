#include "xmc/hw/lock.h"

#include <pico/sem.h>
#include <pico/stdlib.h>
#include <stdlib.h>

static const int NUM_SPINLOCKS = 16;
static uint32_t used_spinlocks = 0;

typedef struct {
  spin_lock_t *hw_lock;
  uint32_t irqs;
  int id;
} spin_lock_hw_t;

#if defined(__cplusplus)
extern "C" {
#endif

xmc_status_t xmc_spinlock_init(xmc_spinlock_t *lock) {
  int id = __builtin_ffs(~used_spinlocks) - 1;
  if (id < 0 || NUM_SPINLOCKS <= id) {
    XMC_ERR_RET(XMC_ERR_SPINLOCK_INIT_FAILED);
  }

  spin_lock_hw_t *hw = (spin_lock_hw_t *)malloc(sizeof(spin_lock_hw_t));
  hw->hw_lock = spin_lock_init(id);
  hw->id = id;
  lock->handle = hw;
  spin_lock_claim(id);
  used_spinlocks |= (1U << id);
  return XMC_OK;
}

void xmc_spinlock_deinit(xmc_spinlock_t *lock) {
  spin_lock_hw_t *hw = (spin_lock_hw_t *)(lock->handle);
  if (!hw) return;
  spin_lock_unclaim(hw->id);
  used_spinlocks &= ~(1U << hw->id);
  free(hw);
  lock->handle = NULL;
}

void xmc_spinlock_get(xmc_spinlock_t *lock) {
  spin_lock_hw_t *hw = (spin_lock_hw_t *)(lock->handle);
  if (!hw) return;
  hw->irqs = spin_lock_blocking(hw->hw_lock);
}

void xmc_spinlock_release(xmc_spinlock_t *lock) {
  spin_lock_hw_t *hw = (spin_lock_hw_t *)(lock->handle);
  if (!hw) return;
  spin_unlock(hw->hw_lock, hw->irqs);
}

xmc_status_t xmc_semaphore_init(xmc_semaphore_t *sem) {
  semaphore_t *sem_handle = (semaphore_t *)malloc(sizeof(semaphore_t));
  if (!sem_handle) {
    return XMC_ERR_SEMAPHORE_INIT_FAILED;
  }
  sem->handle = sem_handle;
  sem_init(sem_handle, 1, 1);
  return XMC_OK;
}

void xmc_semaphore_deinit(xmc_semaphore_t *sem) {
  free(sem->handle);
  sem->handle = NULL;
}

void xmc_semaphore_take(xmc_semaphore_t *sem) {
  sem_acquire_blocking((semaphore_t *)(sem->handle));
}

bool xmc_semaphore_try_take(xmc_semaphore_t *sem) {
  return sem_try_acquire((semaphore_t *)(sem->handle));
}

void xmc_semaphore_give(xmc_semaphore_t *sem) {
  sem_release((semaphore_t *)(sem->handle));
}

#if defined(__cplusplus)
}
#endif
