#include "xmc/app.h"
#include "xmc/display.h"
#include "xmc/hw/timer.h"
#include "xmc/gfx.hpp"

#include <stdint.h>

int r_counter = 0, g_counter = 32767, b_counter = 65535;
float x = XMC_DISPLAY_WIDTH / 2, y = XMC_DISPLAY_HEIGHT / 2;
float dx = 1, dy = 1.11;
int frame = 0;

xmc::Sprite444 frame_buffer(XMC_DISPLAY_WIDTH, XMC_DISPLAY_HEIGHT);

void xmc_app_setup() {}

void xmc_app_loop() {
  r_counter = (r_counter + 1100) % 65536;
  g_counter = (g_counter + 1200) % 65536;
  b_counter = (b_counter + 1300) % 65536;
  int r = r_counter >> 11;
  int g = g_counter >> 11;
  int b = b_counter >> 11;
  if (r >= 16) r = 31 - r;
  if (g >= 16) g = 31 - g;
  if (b >= 16) b = 31 - b;
  x += dx;
  y += dy;
  if (x < 0 || x >= XMC_DISPLAY_WIDTH) {
    dx = -dx;
    x += dx;
  }
  if (y < 0 || y >= XMC_DISPLAY_HEIGHT) {
    dy = -dy;
    y += dy;
  }
  uint32_t color = ((r << 8) | (g << 4) | b);
  //if (740 <= frame && frame <= 743 ) {
  //  xmc_display_fill_rect((int)x - 32, 0, 64, 6, color);
  //}
  //else {
  //  xmc_display_fill_rect((int)x - 32, (int)y - 32, 64, 64, color);
  //}
  //xmc_display_fill_rect(0, 0, 240, 240, color);
  frame_buffer.fill_rect((int)x - 32, (int)y - 32, 64, 64, color);
  frame_buffer.start_transfer_to_display(0, 0);
  frame_buffer.complete_transfer();

  frame++;
}
