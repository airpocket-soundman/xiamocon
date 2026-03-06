#include "xmc/input.h"
#include "xmc/ioex.h"

#include <stddef.h>
#include <stdint.h>

static xmc_button_t curr_state = 0;
static xmc_button_t last_state = 0;

void xmc_input_init() {
  int num_game_buttons =
      sizeof(XMC_IOEX_GAME_BUTTON_PINS) / sizeof(XMC_IOEX_GAME_BUTTON_PINS[0]);
  for (size_t i = 0; i < num_game_buttons; i++) {
    xmc_ioex_set_dir(XMC_IOEX_GAME_BUTTON_PINS[i], false);
  }
  xmc_ioex_set_dir(XMC_IOEX_PIN_BTN_FUNC, false);
  curr_state = 0;
  last_state = 0;
}

void xmc_input_deinit() {
  curr_state = 0;
  last_state = 0;
}

xmc_status_t xmc_input_service() {
  uint16_t tmp;
  XMC_ERR_RET(xmc_ioex_read_all(&tmp));
  last_state = curr_state;
  curr_state = ~tmp & XMC_BUTTON_ALL;
  return XMC_OK;
}

xmc_button_t xmc_input_get_state() { return curr_state; }

bool xmc_input_is_pressed(xmc_button_t button) {
  return (curr_state & button) != 0;
}

bool xmc_input_was_pressed(xmc_button_t button) {
  return ((curr_state & button) != 0) && ((last_state & button) == 0);
}

bool xmc_input_was_released(xmc_button_t button) {
  return ((curr_state & button) == 0) && ((last_state & button) != 0);
}
