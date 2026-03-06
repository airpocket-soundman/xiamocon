#ifndef XMC_IOEX_H
#define XMC_IOEX_H

#include "xmc/xmc_common.h"

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
  XMC_IOEX_PIN_USER_0 = 0,
  XMC_IOEX_PIN_USER_1 = 1,
  XMC_IOEX_PIN_USER_2 = 2,
  XMC_IOEX_PIN_USER_3 = 3,
  XMC_IOEX_PIN_PERI_EN = 4,
  XMC_IOEX_PIN_DISPLAY_RESET = 5,
  XMC_IOEX_PIN_INT_MUTE = 6,
  XMC_IOEX_PIN_BTN_MENU = 7,
  XMC_IOEX_PIN_BTN_A = 8,
  XMC_IOEX_PIN_BTN_B = 9,
  XMC_IOEX_PIN_BTN_X = 10,
  XMC_IOEX_PIN_BTN_Y = 11,
  XMC_IOEX_PIN_BTN_UP = 12,
  XMC_IOEX_PIN_BTN_DOWN = 13,
  XMC_IOEX_PIN_BTN_LEFT = 14,
  XMC_IOEX_PIN_BTN_RIGHT = 15,
} xmc_ioex_pin_t;

static const xmc_ioex_pin_t XMC_IOEX_GAME_BUTTON_PINS[] = {
    XMC_IOEX_PIN_BTN_A,    XMC_IOEX_PIN_BTN_B,     XMC_IOEX_PIN_BTN_X,
    XMC_IOEX_PIN_BTN_Y,    XMC_IOEX_PIN_BTN_UP,    XMC_IOEX_PIN_BTN_DOWN,
    XMC_IOEX_PIN_BTN_LEFT, XMC_IOEX_PIN_BTN_RIGHT,
};

xmc_status_t xmc_ioex_init();
xmc_status_t xmc_ioex_deinit();
xmc_status_t xmc_ioex_set_dir_masked(int port, uint8_t mask, uint8_t value);
xmc_status_t xmc_ioex_write_masked(int port, uint8_t mask, uint8_t value);
xmc_status_t xmc_ioex_read_masked(int port, uint8_t mask, uint8_t *value);

static inline xmc_status_t xmc_ioex_set_dir(xmc_ioex_pin_t pin, bool output) {
  int port = pin / 8;
  int bit = pin % 8;
  return xmc_ioex_set_dir_masked(port, 1 << bit, output ? (1 << bit) : 0);
}

static inline xmc_status_t xmc_ioex_write(xmc_ioex_pin_t pin, bool value) {
  int port = pin / 8;
  int bit = pin % 8;
  return xmc_ioex_write_masked(port, 1 << bit, value ? (1 << bit) : 0);
}

static inline xmc_status_t xmc_ioex_read(xmc_ioex_pin_t pin, bool *value) {
  int port = pin / 8;
  int bit = pin % 8;
  uint8_t temp;
  xmc_status_t status = xmc_ioex_read_masked(port, 1 << bit, &temp);
  if (status == XMC_OK) {
    *value = (temp != 0);
  }
  return status;
}

xmc_status_t xmc_ioex_read_all(uint16_t *value);

#if defined(__cplusplus)
}
#endif

#endif
