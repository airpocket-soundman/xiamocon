#ifndef XMC_HW_TIMER_HPP
#define XMC_HW_TIMER_HPP

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

uint64_t xmc_get_time_ms();
uint64_t xmc_get_time_us();
void xmc_sleep_ms(uint32_t ms);
void xmc_sleep_us(uint32_t us);

#if defined(__cplusplus)
}
#endif

#endif  // XMC_HW_TIMER_HPP
