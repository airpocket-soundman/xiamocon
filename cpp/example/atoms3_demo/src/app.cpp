#include "xmc/app.hpp"
#include "xmc/display.hpp"
#include "xmc/gfx.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/input.hpp"

#include <stdint.h>

using namespace xmc;

// 128x128 frame buffer (RGB444)
static Sprite fb = createSprite444(display::WIDTH, display::HEIGHT);

// Ball state
static float bx = display::WIDTH  / 2.0f;
static float by = display::HEIGHT / 2.0f;
static float bdx = 1.8f;
static float bdy = 1.2f;

// Color cycling triggered by btn A
static const uint32_t COLORS[] = {
    0x00F,  // blue
    0xF00,  // red
    0x0F0,  // green
    0xFF0,  // yellow
    0xF0F,  // magenta
    0x0FF,  // cyan
    0xFFF,  // white
};
static const int NUM_COLORS = (int)(sizeof(COLORS) / sizeof(COLORS[0]));
static int s_color_idx = 0;

AppConfig xmc::appGetConfig() {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = display::InterfaceFormat::RGB444;
  cfg.speakerSampleFormat = audio::SampleFormat::LINEAR_PCM_S16_MONO;
  cfg.speakerSampleRateHz = 22050;
  return cfg;
}

void xmc::appSetup() {
  fb->clear(0x000);
}

void xmc::appLoop() {
  // Btn A: cycle ball color
  if (input::wasPressed(input::Button::A)) {
    s_color_idx = (s_color_idx + 1) % NUM_COLORS;
  }

  uint32_t ball_color = COLORS[s_color_idx];

  // Move ball
  bx += bdx;
  by += bdy;
  if (bx < 8.0f || bx >= display::WIDTH  - 8.0f) { bdx = -bdx; bx += bdx * 2; }
  if (by < 8.0f || by >= display::HEIGHT - 8.0f) { bdy = -bdy; by += bdy * 2; }

  // Draw frame
  fb->completeTransfer();
  fb->clear(0x000);

  // Ball
  fb->fillRect((int)bx - 8, (int)by - 8, 16, 16, ball_color);

  // Button indicator: bright square in corner when btn A is held
  if (input::isPressed(input::Button::A)) {
    fb->fillRect(display::WIDTH - 14, display::HEIGHT - 14, 12, 12, 0xFFF);
  }

  fb->startTransferToDisplay(0, 0);

  sleepMs(16);  // ~60 FPS
}
