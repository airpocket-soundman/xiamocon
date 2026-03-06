#ifndef XMC_HW_LOCK_HPP
#define XMC_HW_LOCK_HPP

#include "xmc/hw/hw_common.h"

#include <stdbool.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/** Opaque handle for a spinlock. */
typedef struct {
  void *handle;
} xmc_spinlock_t;

/** Opaque handle for a semaphore. */
typedef struct {
  void *handle;
} xmc_semaphore_t;

/**
 * Initialize a spinlock.
 * @param lock Pointer to the spinlock to initialize.
 * @return XMC_OK if the spinlock was successfully initialized.
 */
xmc_status_t xmc_spinlock_init(xmc_spinlock_t *lock);

/**
 *  Deinitialize a spinlock.
 * @param lock Pointer to the spinlock to deinitialize.
 */
void xmc_spinlock_deinit(xmc_spinlock_t *lock);

/**
 * Acquire a spinlock.
 * @param lock Pointer to the spinlock to acquire.
 */
void xmc_spinlock_get(xmc_spinlock_t *lock);

/**
 * Release a spinlock.
 * @param lock Pointer to the spinlock to release.
 */
void xmc_spinlock_release(xmc_spinlock_t *lock);

/**
 * Initialize a semaphore.
 * @param sem Pointer to the semaphore to initialize.
 * @return XMC_OK if the semaphore was successfully initialized.
 */
xmc_status_t xmc_semaphore_init(xmc_semaphore_t *sem);

/**
 * Deinitialize a semaphore.
 * @param sem Pointer to the semaphore to deinitialize.
 */
void xmc_semaphore_deinit(xmc_semaphore_t *sem);

/**
 * Take a semaphore.
 * @param sem Pointer to the semaphore to take.
 */
void xmc_semaphore_take(xmc_semaphore_t *sem);

/**
 * Try to take a semaphore.
 * @param sem Pointer to the semaphore to try to take.
 * @return true if the semaphore was successfully taken, false otherwise.
 */
bool xmc_semaphore_try_take(xmc_semaphore_t *sem);

/**
 * Give a semaphore.
 * @param sem Pointer to the semaphore to give.
 */
void xmc_semaphore_give(xmc_semaphore_t *sem);

#if defined(__cplusplus)
}
#endif

#endif
