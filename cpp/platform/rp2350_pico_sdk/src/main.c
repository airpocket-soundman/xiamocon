#include "xmc/app.h"
#include "xmc/system.h"

#include <hardware/clocks.h>
#include <hardware/vreg.h>
#include <pico/stdlib.h>

int main() {
  xmc_sys_init();
  xmc_app_setup();
  while (1) {
    xmc_app_loop();
  }
  return 0;
}
