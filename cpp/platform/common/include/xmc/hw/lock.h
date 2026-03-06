#ifndef XMC_HW_LOCK_HPP
#define XMC_HW_LOCK_HPP

#include "xmc/hw/hw_common.h"

#include <stdint.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
  void *handle;
} xmc_spinlock_t;

typedef struct {
  void *handle;
} xmc_semaphore_t;

xmc_status_t xmc_spinlock_init(xmc_spinlock_t *lock);
void xmc_spinlock_deinit(xmc_spinlock_t *lock);
void xmc_spinlock_get(xmc_spinlock_t *lock);
void xmc_spinlock_release(xmc_spinlock_t *lock);

xmc_status_t xmc_semaphore_init(xmc_semaphore_t *sem);
void xmc_semaphore_deinit(xmc_semaphore_t *sem);
void xmc_semaphore_take(xmc_semaphore_t *sem);
bool xmc_semaphore_try_take(xmc_semaphore_t *sem);
void xmc_semaphore_give(xmc_semaphore_t *sem);

#if defined(__cplusplus)
}
#endif

#endif
