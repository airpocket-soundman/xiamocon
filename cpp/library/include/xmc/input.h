/**
 * @file input.h
 * @brief Input handling for XMC library
 */

#ifndef XMC_INPUT_H
#define XMC_INPUT_H

#include "xmc/ioex.h"
#include "xmc/xmc_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Button definitions.
 */
typedef enum {
  XMC_BUTTON_A = (1 << XMC_IOEX_PIN_BTN_A),
  XMC_BUTTON_B = (1 << XMC_IOEX_PIN_BTN_B),
  XMC_BUTTON_X = (1 << XMC_IOEX_PIN_BTN_X),
  XMC_BUTTON_Y = (1 << XMC_IOEX_PIN_BTN_Y),
  XMC_BUTTON_UP = (1 << XMC_IOEX_PIN_BTN_UP),
  XMC_BUTTON_DOWN = (1 << XMC_IOEX_PIN_BTN_DOWN),
  XMC_BUTTON_LEFT = (1 << XMC_IOEX_PIN_BTN_LEFT),
  XMC_BUTTON_RIGHT = (1 << XMC_IOEX_PIN_BTN_RIGHT),
  XMC_BUTTON_FUNC = (1 << XMC_IOEX_PIN_BTN_FUNC),
  XMC_BUTTON_ALL = (XMC_BUTTON_A | XMC_BUTTON_B | XMC_BUTTON_X | XMC_BUTTON_Y |
                    XMC_BUTTON_UP | XMC_BUTTON_DOWN | XMC_BUTTON_LEFT |
                    XMC_BUTTON_RIGHT | XMC_BUTTON_FUNC),
} xmc_button_t;

/**
 * Initialize the input system. This will configure the IO expander pins for
 * reading button states and initialize any necessary data structures.
 */
void xmc_input_init();

/**
 * Deinitialize the input system.
 */
void xmc_input_deinit();

/**
 * Service the input system. This should be called periodically (e.g. once per
 * frame) to update the button states. This function reads the current state of
 * the buttons from the IO expander and updates internal state to track which
 * buttons are currently pressed, which buttons were just pressed, and which
 * buttons were just released. The current button state can then be queried
 * using the other input functions.
 * @return XMC_OK if the input state was successfully updated.
 */
xmc_status_t xmc_input_service();

/**
 * Get the current state of all buttons. The return value is a bitmask where
 * each bit corresponds to a button (1 for pressed, 0 for not pressed). The
 * buttons are defined in the xmc_button_t enum, so you can check if a specific
 * button is pressed by performing a bitwise AND with the corresponding button
 * mask. For example, to check if the A button is pressed, you can do:
 * if (xmc_input_get_state() & XMC_BUTTON_A) { ... }
 * @return A bitmask representing the current state of all buttons. Each bit
 * corresponds to a button (1 for pressed, 0 for not pressed). The buttons are
 * defined in the xmc_button_t enum.
 */
xmc_button_t xmc_input_get_state();

/**
 * Check if a specific button is currently pressed. This is a convenience
 * function that checks if the specified button's bit is set in the current
 * button state.
 * @param button The button to check (use the xmc_button_t enum values).
 * @return true if the specified button is currently pressed, false otherwise.
 */
bool xmc_input_is_pressed(xmc_button_t button);

/**
 * Check if a specific button was just pressed. This returns true if the button
 * is currently pressed but was not pressed in the previous state. This can be
 * used to detect the moment a button is pressed, as opposed to it being held
 * down.
 * @param button The button to check (use the xmc_button_t enum values).
 * @return true if the specified button was just pressed, false otherwise.
 */
bool xmc_input_was_pressed(xmc_button_t button);

/**
 * Check if a specific button was just released. This returns true if the button
 * is currently not pressed but was pressed in the previous state. This can be
 * used to detect the moment a button is released.
 * @param button The button to check (use the xmc_button_t enum values).
 * @return true if the specified button was just released, false otherwise.
 */
bool xmc_input_was_released(xmc_button_t button);

#if defined(__cplusplus)
}
#endif

#endif