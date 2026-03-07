/**
 * @file timer.h
 * @brief Timer hardware interface
 */

#ifndef XMC_HW_TIMER_HPP
#define XMC_HW_TIMER_HPP

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Get the current time in milliseconds since an arbitrary epoch. The epoch and
 * resolution are implementation-defined, but the value will always be
 * increasing and will wrap around on overflow.
 */
uint64_t xmc_get_time_ms();

/**
 * Get the current time in microseconds since an arbitrary epoch. The epoch and
 * resolution are implementation-defined, but the value will always be
 * increasing and will wrap around on overflow.
 */
uint64_t xmc_get_time_us();

/**
 * Sleep for the specified number of milliseconds.
 * @param ms The number of milliseconds to sleep.
 */
void xmc_sleep_ms(uint32_t ms);

/**
 * Sleep for the specified number of microseconds.
 * @param us The number of microseconds to sleep.
 */
void xmc_sleep_us(uint32_t us);

#if defined(__cplusplus)
}
#endif

#endif  // XMC_HW_TIMER_HPP
