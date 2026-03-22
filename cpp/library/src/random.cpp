#include "xmc/random.hpp"

#include <stdlib.h>

namespace xmc {

uint32_t randomU32() { return (uint32_t)rand(); }

}  // namespace xmc
