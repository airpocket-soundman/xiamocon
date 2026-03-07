#include "xmc/hw/timer.h"

#include <pico/stdlib.h>

uint64_t xmc_get_time_ms() { return to_ms_since_boot(get_absolute_time()); }
uint64_t xmc_get_time_us() { return to_us_since_boot(get_absolute_time()); }
void xmc_sleep_ms(uint32_t ms) { sleep_ms(ms); }
void xmc_sleep_us(uint32_t us) { sleep_us(us); }
