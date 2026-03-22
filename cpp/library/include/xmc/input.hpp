/**
 * @file input.hpp
 * @brief Input handling for XMC library
 */

#ifndef XMC_INPUT_HPP
#define XMC_INPUT_HPP

#include "xmc/ioex.hpp"
#include "xmc/xmc_common.hpp"

namespace xmc::input {

/**
 * Button definitions.
 */
enum class Button : uint16_t {
  /** No button pressed */
  NONE = 0,

  /** Button A */
  A = (1 << (int)ioex::Pin::BTN_A),

  /** Button B */
  B = (1 << (int)ioex::Pin::BTN_B),

  /** Button X */
  X = (1 << (int)ioex::Pin::BTN_X),

  /** Button Y */
  Y = (1 << (int)ioex::Pin::BTN_Y),

  /** Button Up */
  UP = (1 << (int)ioex::Pin::BTN_UP),

  /** Button Down */
  DOWN = (1 << (int)ioex::Pin::BTN_DOWN),

  /** Button Left */
  LEFT = (1 << (int)ioex::Pin::BTN_LEFT),

  /** Button Right */
  RIGHT = (1 << (int)ioex::Pin::BTN_RIGHT),

  /** Function Button */
  FUNC = (1 << (int)ioex::Pin::BTN_FUNC),

  /** Bitmask for all buttons */
  ANY = (A | B | X | Y | UP | DOWN | LEFT | RIGHT | FUNC),
};
XMC_ENUM_FLAGS(Button, uint16_t)

/**
 * Initialize the input system. This will configure the IO expander pins for
 * reading button states and initialize any necessary data structures.
 */
void init();

/**
 * Deinitialize the input system.
 */
void deinit();

/**
 * Service the input system. This should be called periodically (e.g. once per
 * frame) to update the button states. This function reads the current state of
 * the buttons from the IO expander and updates internal state to track which
 * buttons are currently pressed, which buttons were just pressed, and which
 * buttons were just released. The current button state can then be queried
 * using the other input functions.
 * @return XMC_OK if the input state was successfully updated.
 */
XmcStatus service();

/**
 * Get the current state of all buttons. The return value is a bitmask where
 * each bit corresponds to a button (1 for pressed, 0 for not pressed). The
 * buttons are defined in the Button enum, so you can check if a specific
 * button is pressed by performing a bitwise AND with the corresponding button
 * mask. For example, to check if the A button is pressed, you can do:
 * if (getState() & A) { ... }
 * @return A bitmask representing the current state of all buttons. Each bit
 * corresponds to a button (1 for pressed, 0 for not pressed). The buttons are
 * defined in the Button enum.
 */
Button getState();

/**
 * Check if a specific button is currently pressed. This is a convenience
 * function that checks if the specified button's bit is set in the current
 * button state.
 * @param button The button to check (use the Button enum values).
 * @return true if the specified button is currently pressed, false otherwise.
 */
bool isPressed(Button button);

/**
 * Check if a specific button was just pressed. This returns true if the button
 * is currently pressed but was not pressed in the previous state. This can be
 * used to detect the moment a button is pressed, as opposed to it being held
 * down.
 * @param button The button to check (use the Button enum values).
 * @return true if the specified button was just pressed, false otherwise.
 */
bool wasPressed(Button button);

/**
 * Check if a specific button was just released. This returns true if the button
 * is currently not pressed but was pressed in the previous state. This can be
 * used to detect the moment a button is released.
 * @param button The button to check (use the Button enum values).
 * @return true if the specified button was just released, false otherwise.
 */
bool wasReleased(Button button);

}  // namespace xmc::input

#endif
