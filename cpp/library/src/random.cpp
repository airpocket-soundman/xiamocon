#include "xmc/random.h"

#include <stdlib.h>

uint32_t xmc_randomNextU32() {
  return (uint32_t)rand();
}


