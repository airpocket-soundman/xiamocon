#include "xmc/app.hpp"
#include "xmc/audio.hpp"
#include "xmc/display.hpp"
#include "xmc/gfx.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/input.hpp"
#include "xmc/speaker.hpp"

#include <math.h>
#include <stdint.h>

namespace {

using namespace xmc;
using namespace xmc::audio;

constexpr int SCREEN_W = display::WIDTH;
constexpr int SCREEN_H = display::HEIGHT;
constexpr uint32_t SAMPLE_RATE_HZ = 22050;
constexpr int NUM_TONES = 4;

constexpr int PADDLE_W = 44;
constexpr int PADDLE_H = 6;
constexpr int PADDLE_Y = SCREEN_H - 18;
constexpr int PADDLE_SPEED = 5;

constexpr int BALL_SIZE = 4;

constexpr int BRICK_COLS = 10;
constexpr int BRICK_ROWS = 6;
constexpr int BRICK_GAP = 2;
constexpr int BRICK_MARGIN_X = 10;
constexpr int BRICK_TOP = 24;
constexpr int BRICK_H = 10;
constexpr int BRICK_W =
    (SCREEN_W - BRICK_MARGIN_X * 2 - BRICK_GAP * (BRICK_COLS - 1)) / BRICK_COLS;

constexpr uint16_t rgb444(int r, int g, int b) {
  return static_cast<uint16_t>(((r & 0xF) << 8) | ((g & 0xF) << 4) | (b & 0xF));
}

constexpr uint16_t COLOR_BG = rgb444(0, 0, 0);
constexpr uint16_t COLOR_WALL = rgb444(1, 1, 2);
constexpr uint16_t COLOR_PADDLE = rgb444(2, 15, 15);
constexpr uint16_t COLOR_BALL = rgb444(15, 15, 15);
constexpr uint16_t COLOR_SERVE = rgb444(15, 15, 0);
constexpr uint16_t COLOR_CLEAR = rgb444(0, 10, 0);
constexpr uint16_t COLOR_GAME_OVER = rgb444(10, 0, 0);

struct Brick {
  int x;
  int y;
  bool alive;
  uint16_t color;
};

enum class GameState {
  Serve,
  Playing,
  Clear,
  GameOver,
};

Sprite screen = xmc::createSprite444(SCREEN_W, SCREEN_H);
Brick bricks[BRICK_ROWS][BRICK_COLS];
Mixer mixer(NUM_TONES);
Tone tones[NUM_TONES];

GameState game_state = GameState::Serve;
int next_tone_index = 0;

int paddle_x = (SCREEN_W - PADDLE_W) / 2;

float ball_x = 0.0f;
float ball_y = 0.0f;
float ball_vx = 0.0f;
float ball_vy = 0.0f;

int bricks_left = 0;

void play_tone(uint8_t note, uint32_t lenMs, audio::Waveform waveform,
               uint8_t velocity, uint16_t attackMs, uint16_t decayMs,
               uint16_t sustain, uint16_t releaseMs, int32_t sweep_delta,
               uint32_t sweepPeriodMs) {
  audio::Tone &tone = tones[next_tone_index];
  next_tone_index = (next_tone_index + 1) % NUM_TONES;

  tone.setWaveform(waveform);
  tone.setVelocity(velocity);
  tone.setEnvelope(attackMs, decayMs, sustain, releaseMs);
  tone.setSweep(sweep_delta, sweepPeriodMs);
  tone.noteOn(note, lenMs);
}

void sfx_launch() {
  play_tone(76, 90, Waveform::SQUARE, 96, 0, 60, 140, 40, 2800, 7);
}

void sfx_wall_bounce() {
  play_tone(82, 28, Waveform::SQUARE, 72, 0, 24, 96, 18, 0, 10);
}

void sfx_paddle_bounce() {
  play_tone(62, 45, Waveform::TRIANGLE, 86, 0, 30, 120, 24, 900, 8);
}

void sfx_brick_break() {
  play_tone(72, 55, Waveform::SQUARE, 88, 0, 40, 96, 28, -1700, 10);
}

void sfx_clear() {
  play_tone(76, 130, Waveform::TRIANGLE, 104, 0, 80, 160, 40, 2200, 9);
  play_tone(83, 150, Waveform::SINE, 90, 0, 100, 140, 50, 1100, 12);
}

void sfx_game_over() {
  play_tone(45, 180, Waveform::SAWTOOTH, 92, 0, 120, 110, 70, -2400, 12);
}

bool intersects_rect(int ax, int ay, int aw, int ah, int bx, int by, int bw,
                     int bh) {
  return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

void reset_ball_on_paddle() {
  ball_x = static_cast<float>(paddle_x + PADDLE_W / 2 - BALL_SIZE / 2);
  ball_y = static_cast<float>(PADDLE_Y - BALL_SIZE - 1);
  ball_vx = 0.0f;
  ball_vy = 0.0f;
}

void build_stage() {
  static constexpr uint16_t row_colors[BRICK_ROWS] = {
      rgb444(15, 4, 4), rgb444(15, 8, 2),  rgb444(15, 12, 2),
      rgb444(6, 15, 4), rgb444(4, 10, 15), rgb444(10, 6, 15),
  };

  bricks_left = BRICK_ROWS * BRICK_COLS;
  for (int row = 0; row < BRICK_ROWS; ++row) {
    for (int col = 0; col < BRICK_COLS; ++col) {
      Brick &brick = bricks[row][col];
      brick.x = BRICK_MARGIN_X + col * (BRICK_W + BRICK_GAP);
      brick.y = BRICK_TOP + row * (BRICK_H + BRICK_GAP);
      brick.alive = true;
      brick.color = row_colors[row];
    }
  }
}

void reset_game() {
  paddle_x = (SCREEN_W - PADDLE_W) / 2;
  build_stage();
  reset_ball_on_paddle();
  game_state = GameState::Serve;
}

void launch_ball(input::Button buttons) {
  float dir = 0.0f;
  if (hasFlag(buttons, input::Button::LEFT)) {
    dir = -1.0f;
  }
  if (hasFlag(buttons, input::Button::RIGHT)) {
    dir = 1.0f;
  }
  ball_vx = dir * 1.4f;
  ball_vy = -2.2f;
  game_state = GameState::Playing;
  sfx_launch();
}

void move_paddle(input::Button buttons) {
  if (hasFlag(buttons, input::Button::LEFT)) {
    paddle_x -= PADDLE_SPEED;
  }
  if (hasFlag(buttons, input::Button::RIGHT)) {
    paddle_x += PADDLE_SPEED;
  }
  if (paddle_x < 0) {
    paddle_x = 0;
  }
  if (paddle_x > SCREEN_W - PADDLE_W) {
    paddle_x = SCREEN_W - PADDLE_W;
  }
}

void update_ball() {
  const float prev_x = ball_x;
  const float prev_y = ball_y;

  ball_x += ball_vx;
  ball_y += ball_vy;

  if (ball_x < 0.0f) {
    ball_x = 0.0f;
    ball_vx = fabsf(ball_vx);
    sfx_wall_bounce();
  } else if (ball_x + BALL_SIZE > SCREEN_W) {
    ball_x = static_cast<float>(SCREEN_W - BALL_SIZE);
    ball_vx = -fabsf(ball_vx);
    sfx_wall_bounce();
  }

  if (ball_y < 0.0f) {
    ball_y = 0.0f;
    ball_vy = fabsf(ball_vy);
    sfx_wall_bounce();
  }

  if (ball_y > SCREEN_H) {
    game_state = GameState::GameOver;
    sfx_game_over();
    return;
  }

  const int bi_x = static_cast<int>(ball_x);
  const int bi_y = static_cast<int>(ball_y);

  if (ball_vy > 0.0f &&
      intersects_rect(bi_x, bi_y, BALL_SIZE, BALL_SIZE, paddle_x, PADDLE_Y,
                      PADDLE_W, PADDLE_H)) {
    ball_y = static_cast<float>(PADDLE_Y - BALL_SIZE - 1);
    const float hit =
        ((ball_x + BALL_SIZE * 0.5f) - (paddle_x + PADDLE_W * 0.5f)) /
        (PADDLE_W * 0.5f);
    ball_vx = hit * 2.4f;
    ball_vy = -fabsf(ball_vy);
    const float speed = sqrtf(ball_vx * ball_vx + ball_vy * ball_vy);
    if (speed < 3.2f) {
      const float scale = 3.2f / speed;
      ball_vx *= scale;
      ball_vy *= scale;
    }
    sfx_paddle_bounce();
  }

  for (int row = 0; row < BRICK_ROWS; ++row) {
    for (int col = 0; col < BRICK_COLS; ++col) {
      Brick &brick = bricks[row][col];
      if (!brick.alive) {
        continue;
      }
      if (!intersects_rect(bi_x, bi_y, BALL_SIZE, BALL_SIZE, brick.x, brick.y,
                           BRICK_W, BRICK_H)) {
        continue;
      }

      brick.alive = false;
      --bricks_left;
      sfx_brick_break();

      const float prev_right = prev_x + BALL_SIZE;
      const float prev_bottom = prev_y + BALL_SIZE;
      const bool from_left = prev_right <= brick.x;
      const bool from_right = prev_x >= brick.x + BRICK_W;
      const bool from_top = prev_bottom <= brick.y;
      const bool from_bottom = prev_y >= brick.y + BRICK_H;

      if (from_left || from_right) {
        ball_vx = -ball_vx;
      } else if (from_top || from_bottom) {
        ball_vy = -ball_vy;
      } else {
        ball_vy = -ball_vy;
      }

      if (bricks_left <= 0) {
        game_state = GameState::Clear;
        sfx_clear();
      }
      return;
    }
  }
}

void draw_scene() {
  screen->completeTransfer();
  screen->clear(COLOR_BG);

  screen->fillRect(0, 0, SCREEN_W, 2, COLOR_WALL);

  for (int row = 0; row < BRICK_ROWS; ++row) {
    for (int col = 0; col < BRICK_COLS; ++col) {
      const Brick &brick = bricks[row][col];
      if (!brick.alive) {
        continue;
      }
      screen->fillRect(brick.x, brick.y, BRICK_W, BRICK_H, brick.color);
    }
  }

  screen->fillRect(paddle_x, PADDLE_Y, PADDLE_W, PADDLE_H, COLOR_PADDLE);
  screen->fillRect(static_cast<int>(ball_x), static_cast<int>(ball_y),
                    BALL_SIZE, BALL_SIZE, COLOR_BALL);

  if (game_state == GameState::Serve) {
    screen->fillRect(SCREEN_W / 2 - 40, SCREEN_H / 2 - 8, 80, 16, COLOR_SERVE);
  } else if (game_state == GameState::Clear) {
    screen->fillRect(SCREEN_W / 2 - 52, SCREEN_H / 2 - 12, 104, 24,
                      COLOR_CLEAR);
  } else if (game_state == GameState::GameOver) {
    screen->fillRect(SCREEN_W / 2 - 52, SCREEN_H / 2 - 12, 104, 24,
                      COLOR_GAME_OVER);
  }

  screen->drawLastError();
  screen->startTransferToDisplay(0, 0);
}

}  // namespace

AppConfig xmc::appGetConfig() {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = display::InterfaceFormat::RGB444;
  cfg.speakerSampleFormat = SampleFormat::LINEAR_PCM_S16_MONO;
  cfg.speakerSampleRateHz = SAMPLE_RATE_HZ;
  cfg.speakerLatencySamples = 512;
  return cfg;
}

void xmc::appSetup() {
  for (int i = 0; i < NUM_TONES; ++i) {
    tones[i].init(SAMPLE_RATE_HZ);
    mixer.setSource(i, tones[i].getOutputPort());
  }
  speaker::setSourcePort(mixer.getOutputPort());
  speaker::setMuted(false);

  reset_game();
}

void xmc::appLoop() {
  const input::Button buttons = input::getState();

  move_paddle(buttons);

  if (game_state == GameState::Serve) {
    reset_ball_on_paddle();
    if (hasFlag(buttons, input::Button::A | input::Button::B |
                             input::Button::X | input::Button::Y |
                             input::Button::UP)) {
      launch_ball(buttons);
    }
  } else if (game_state == GameState::Playing) {
    update_ball();
  } else {
    if (hasFlag(buttons, input::Button::A | input::Button::B |
                             input::Button::X | input::Button::Y |
                             input::Button::UP)) {
      reset_game();
    }
  }

  draw_scene();
  xmc::sleepMs(16);
}
