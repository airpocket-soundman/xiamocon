#include "xmc/input.h"
#include "xmc/hw/timer.h"
#include "xmc/ioex.h"

#include <stddef.h>
#include <stdint.h>

static uint64_t nextUpdateMs = 0;
static xmc_button_t curr_state = 0;
static xmc_button_t last_state = 0;

void xmc_inputInit() {
  int num_game_buttons =
      sizeof(XMC_IOEX_GAME_BUTTON_PINS) / sizeof(XMC_IOEX_GAME_BUTTON_PINS[0]);
  for (size_t i = 0; i < num_game_buttons; i++) {
    xmc_ioexSetDir(XMC_IOEX_GAME_BUTTON_PINS[i], false);
  }
  xmc_ioexSetDir(XMC_IOEX_PIN_BTN_FUNC, false);
  curr_state = 0;
  last_state = 0;
}

void xmc_inputDeinit() {
  curr_state = 0;
  last_state = 0;
}

XmcStatus xmc_inputService() {
  uint64_t now_ms = xmc_getTimeMs();
  if (now_ms < nextUpdateMs) {
    return XMC_OK;
  }
  nextUpdateMs += 10;
  if (nextUpdateMs < now_ms) {
    nextUpdateMs = now_ms + 10;
  }

  uint16_t tmp;
  if (xmc_ioexTryReadAll(&tmp)) {
    last_state = curr_state;
    curr_state = ~tmp & XMC_BUTTON_ALL;
  }
  return XMC_OK;
}

xmc_button_t xmc_inputGetState() { return curr_state; }

bool xmc_inputIsPressed(xmc_button_t button) {
  return (curr_state & button) != 0;
}

bool xmc_inputWasPressed(xmc_button_t button) {
  return ((curr_state & button) != 0) && ((last_state & button) == 0);
}

bool xmc_inputWasReleased(xmc_button_t button) {
  return ((curr_state & button) == 0) && ((last_state & button) != 0);
}
