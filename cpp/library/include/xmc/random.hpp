/**
 * @file random.h
 * @brief Random number generation for XMC library
 */

#ifndef XMC_RANDOM_H
#define XMC_RANDOM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the next random 32-bit unsigned integer.
 * @return A random 32-bit unsigned integer.
 */
uint32_t xmc_randomNextU32();

#ifdef __cplusplus
}
#endif

#endif
