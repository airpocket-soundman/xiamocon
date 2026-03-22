#include "xmc/input.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/ioex.hpp"

#include <stddef.h>
#include <stdint.h>

namespace xmc::input {

static uint64_t nextUpdateMs = 0;
static Button curr_state = Button::NONE;
static Button last_state = Button::NONE;

void init() {
  int num_game_buttons =
      sizeof(ioex::GAME_BUTTON_PINS) / sizeof(ioex::GAME_BUTTON_PINS[0]);
  for (size_t i = 0; i < num_game_buttons; i++) {
    ioex::setDir(ioex::GAME_BUTTON_PINS[i], false);
  }
  ioex::setDir(ioex::Pin::BTN_FUNC, false);
  curr_state = Button::NONE;
  last_state = Button::NONE;
}

void deinit() {
  curr_state = Button::NONE;
  last_state = Button::NONE;
}

XmcStatus service() {
  uint64_t now_ms = xmc::getTimeMs();
  if (now_ms < nextUpdateMs) {
    return XMC_OK;
  }
  nextUpdateMs += 10;
  if (nextUpdateMs < now_ms) {
    nextUpdateMs = now_ms + 10;
  }

  uint16_t tmp;
  if (ioex::tryReadAll(&tmp)) {
    last_state = curr_state;
    curr_state = ((Button)~tmp) & Button::ANY;
  }
  return XMC_OK;
}

Button getState() { return curr_state; }

bool isPressed(Button button) { return !!(curr_state & button); }

bool wasPressed(Button button) {
  return !!(curr_state & button) && !(last_state & button);
}

bool wasReleased(Button button) {
  return !(curr_state & button) && !!(last_state & button);
}

}  // namespace xmc::input
