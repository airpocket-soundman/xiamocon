/**
 * @file random.h
 * @brief Random number generation for XMC library
 */

#ifndef XMC_RANDOM_H
#define XMC_RANDOM_H

#include <stdint.h>

namespace xmc {

/**
 * Get the next random 32-bit unsigned integer.
 * @return A random 32-bit unsigned integer.
 */
uint32_t randomU32();

}

#endif
