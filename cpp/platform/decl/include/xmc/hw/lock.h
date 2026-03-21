/**
 * @file lock.h
 * @brief Locking primitives for synchronization
 */

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
} SpinLock;

/** Opaque handle for a semaphore. */
typedef struct {
  void *handle;
} Semaphore;

/**
 * Initialize a spinlock.
 * @param lock Pointer to the spinlock to initialize.
 * @return XMC_OK if the spinlock was successfully initialized.
 */
XmcStatus xmc_spinlockInit(SpinLock *lock);

/**
 *  Deinitialize a spinlock.
 * @param lock Pointer to the spinlock to deinitialize.
 */
void xmc_spinlockDeinit(SpinLock *lock);

/**
 * Acquire a spinlock.
 * @param lock Pointer to the spinlock to acquire.
 */
void xmc_spinlockGet(SpinLock *lock);

/**
 * Release a spinlock.
 * @param lock Pointer to the spinlock to release.
 */
void xmc_spinlockRelease(SpinLock *lock);

/**
 * Initialize a semaphore.
 * @param sem Pointer to the semaphore to initialize.
 * @return XMC_OK if the semaphore was successfully initialized.
 */
XmcStatus xmc_semaphoreInit(Semaphore *sem);

/**
 * Deinitialize a semaphore.
 * @param sem Pointer to the semaphore to deinitialize.
 */
void xmc_semaphoreDeinit(Semaphore *sem);

/**
 * Take a semaphore.
 * @param sem Pointer to the semaphore to take.
 */
void xmc_semaphoreTake(Semaphore *sem);

/**
 * Try to take a semaphore.
 * @param sem Pointer to the semaphore to try to take.
 * @return true if the semaphore was successfully taken, false otherwise.
 */
bool xmc_semaphoreTryTake(Semaphore *sem);

/**
 * Give a semaphore.
 * @param sem Pointer to the semaphore to give.
 */
void xmc_semaphoreGive(Semaphore *sem);

#if defined(__cplusplus)
}
#endif

#endif
