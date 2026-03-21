/**
 * @file timer.h
 * @brief Timer hardware interface
 */

#ifndef XMC_HW_TIMER_HPP
#define XMC_HW_TIMER_HPP

#include "xmc/xmc_common.h"

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Timer tick callback function type. This is the type of the callback function
 * that will be called when a timer tick occurs. The callback function will
 * receive a context pointer that can be used to pass user-defined data to the
 * callback function.
 * @param context A user-defined context pointer that was provided when the
 * timer was initialized.
 * @return A boolean value indicating whether the timer should continue running.
 * If the callback returns true, the timer will continue to run and call
 * the callback again after the next interval. If the callback returns
 * false, the timer will be canceled and will not call the callback again.
 */
typedef bool (*TimerTickCb)(void *context);

/**
 * Repeated timer instance structure. This structure represents an instance of a
 * repeated timer. It contains the hardware-specific data, the timer tick
 * callback function, and a context pointer that can be used to pass
 * user-defined data to the callback function.
 */
typedef struct {
  /** Hardware-specific handle for the timer instance */
  void *handle;
  /** Timer tick callback function */
  TimerTickCb tick;
  /** User-defined context pointer */
  void *context;
} RepeatingTimer;

/**
 * Get the current time in milliseconds since an arbitrary epoch. The epoch and
 * resolution are implementation-defined, but the value will always be
 * increasing and will wrap around on overflow.
 */
uint64_t xmc_getTimeMs();

/**
 * Get the current time in microseconds since an arbitrary epoch. The epoch and
 * resolution are implementation-defined, but the value will always be
 * increasing and will wrap around on overflow.
 */
uint64_t xmc_getTimeUs();

/**
 * Sleep for the specified number of milliseconds.
 * @param ms The number of milliseconds to sleep.
 */
void xmc_sleepMs(uint32_t ms);

/**
 * Sleep for the specified number of microseconds.
 * @param us The number of microseconds to sleep.
 */
void xmc_sleepUs(uint32_t us);

/**
 * Initialize a repeated timer instance. This function will set up a timer that
 * calls the specified callback function at regular intervals defined by
 * interval_ms. The callback function will be called with the provided context
 * pointer. The actual interval may not be exactly equal to interval_ms due to
 * hardware limitations, but it will be as close as possible.
 * @param timer The repeated timer instance to initialize.
 * @param interval_ms The desired interval in milliseconds between timer ticks.
 * @param cb The callback function to call on each timer tick.
 * @param context A user-defined context pointer that will be passed to the
 * callback function.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
XmcStatus xmc_timerAddRepeatingMs(RepeatingTimer *timer,
                                        uint32_t interval_ms,
                                        TimerTickCb cb, void *context);

/**
 * Deinitialize a repeated timer instance. This function will stop the timer and
 * free any resources allocated during initialization. After this function is
 * called, the timer instance will no longer be valid and should not be used.
 * @param timer The repeated timer instance to deinitialize.
 * @return XMC_OK on success, or an appropriate error code on failure.
 */
void xmc_timerCancelRepeating(RepeatingTimer *timer);

#if defined(__cplusplus)
}
#endif

#endif  // XMC_HW_TIMER_HPP
