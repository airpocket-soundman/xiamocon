#include "xmc/display.hpp"
#include "xmc/hw/gpio.hpp"
#include "xmc/hw/pins.hpp"
#include "xmc/hw/spi.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/ioex.hpp"

// GC9107 display driver for M5Stack AtomS3 (128x128).
// The GC9107 is largely MIPI-compatible; we add the proprietary preamble
// (0xFE / 0xEF) to unlock extended registers, then use standard commands.

namespace xmc::display {

// AtomS3 backlight pin (active high)
static const int ATOMS3_PIN_BL = 16;

// ---- Command constants (MIPI DCS, compatible with GC9107) ----------------
static const uint8_t CMD_SWRST  = 0x01;
static const uint8_t CMD_SLPOUT = 0x11;
static const uint8_t CMD_NORON  = 0x13;
static const uint8_t CMD_INVOFF = 0x20;
static const uint8_t CMD_INVON  = 0x21;
static const uint8_t CMD_DISPOFF = 0x28;
static const uint8_t CMD_DISPON = 0x29;
static const uint8_t CMD_CASET  = 0x2A;
static const uint8_t CMD_RASET  = 0x2B;
static const uint8_t CMD_RAMWR  = 0x2C;
static const uint8_t CMD_MADCTL = 0x36;
static const uint8_t CMD_COLMOD = 0x3A;

// GC9107 proprietary unlock sequence
static const uint8_t GC9107_INTER_CMD = 0xFE;
static const uint8_t GC9107_INTER_REG = 0xEF;

// ---- Internal state -------------------------------------------------------
static InterfaceFormat s_format = InterfaceFormat::RGB444;
static bool s_in_transaction = false;
static uint8_t s_line_buf[WIDTH * 3];

// ---- Internal helpers -----------------------------------------------------
static XmcStatus begin_command(uint8_t cmd);
static void end_command();
static XmcStatus write_data(const uint8_t *data, uint32_t size);

// ---- Public API -----------------------------------------------------------

XmcStatus init(InterfaceFormat format, int /*rotation*/) {
  s_format = format;

  // Configure SPI control pins
  gpio::setDir(XMC_PIN_DISPLAY_CS, true);
  gpio::setDir(XMC_PIN_DISPLAY_DC, true);
  gpio::write(XMC_PIN_DISPLAY_CS, 1);
  gpio::write(XMC_PIN_DISPLAY_DC, 1);

  // Hardware reset via IO expander (maps to GPIO34 on AtomS3)
  ioex::setDir(ioex::Pin::DISPLAY_RESET, true);
  ioex::write(ioex::Pin::DISPLAY_RESET, 0);
  sleepMs(10);
  ioex::write(ioex::Pin::DISPLAY_RESET, 1);
  sleepMs(120);

  // GC9107: unlock extended registers
  writeCommandNoParam(GC9107_INTER_CMD);
  writeCommandNoParam(GC9107_INTER_REG);

  // Sleep out
  writeCommandNoParam(CMD_SLPOUT);
  sleepMs(120);

  // Normal display mode on
  writeCommandNoParam(CMD_NORON);

  // Memory access control: BGR bit set (GC9107 panel uses BGR order)
  writeCommand1Param(CMD_MADCTL, 0x08);

  // Pixel format
  uint8_t colmod;
  switch (s_format) {
    case InterfaceFormat::RGB444: colmod = 0x53; break;
    case InterfaceFormat::RGB565: colmod = 0x55; break;
    default: return XMC_ERR_DISPLAY_UNSUPPORTED_FORMAT;
  }
  writeCommand1Param(CMD_COLMOD, colmod);

  // Column address: 0 – 127
  writeCommand4Params(CMD_CASET, 0x00, 0x00, 0x00, (uint8_t)(WIDTH - 1));

  // Row address: 0 – 127
  writeCommand4Params(CMD_RASET, 0x00, 0x00, 0x00, (uint8_t)(HEIGHT - 1));

  // GC9107 display function control
  {
    uint8_t p[] = {0x00, 0x00};
    writeCommand(0xB6, p, 2);
  }

  // GC9107 positive gamma
  {
    uint8_t p[] = {0x45, 0x09, 0x08, 0x08, 0x26, 0x2A};
    writeCommand(0xF0, p, sizeof(p));
  }

  // GC9107 negative gamma
  {
    uint8_t p[] = {0x43, 0x70, 0x72, 0x36, 0x37, 0x6F};
    writeCommand(0xF1, p, sizeof(p));
  }

  // Display inversion on (required for correct colors on GC9107)
  writeCommandNoParam(CMD_INVON);

  // Clear the screen first
  fillRect(0, 0, WIDTH, HEIGHT, 0);

  // Display on
  writeCommandNoParam(CMD_DISPON);
  sleepMs(20);

  // Enable backlight
  gpio::setDir(ATOMS3_PIN_BL, true);
  gpio::write(ATOMS3_PIN_BL, 1);

  return XMC_OK;
}

XmcStatus deinit() {
  gpio::write(ATOMS3_PIN_BL, 0);
  writeCommandNoParam(CMD_DISPOFF);
  gpio::write(XMC_PIN_DISPLAY_CS, 1);
  gpio::write(XMC_PIN_DISPLAY_DC, 1);
  ioex::write(ioex::Pin::DISPLAY_RESET, 0);
  return XMC_OK;
}

XmcStatus clear(uint32_t color) {
  return fillRect(0, 0, WIDTH, HEIGHT, color);
}

XmcStatus fillRect(int x, int y, int width, int height, uint32_t color) {
  if (x < 0) { width  += x; x = 0; }
  if (y < 0) { height += y; y = 0; }
  if (x + width  > WIDTH)  width  = WIDTH  - x;
  if (y + height > HEIGHT) height = HEIGHT - y;
  if (width <= 0 || height <= 0) return XMC_OK;

  int line_bytes;
  switch (s_format) {
    case InterfaceFormat::RGB444: {
      line_bytes = width * 3 / 2;
      for (int i = 0; i < line_bytes; i += 3) {
        s_line_buf[i + 0] = (color >> 4) & 0xFF;
        s_line_buf[i + 1] = ((color & 0x0F) << 4) | ((color >> 8) & 0x0F);
        s_line_buf[i + 2] =  color & 0xFF;
      }
    } break;
    case InterfaceFormat::RGB565: {
      line_bytes = width * 2;
      uint16_t *buf = (uint16_t *)s_line_buf;
      for (int i = 0; i < width; i++) buf[i] = (uint16_t)color;
    } break;
    default: return XMC_ERR_DISPLAY_UNSUPPORTED_FORMAT;
  }

  for (int row = 0; row < height; row++) {
    XMC_ERR_RET(setWindow(x, y + row, width, 1));
    XMC_ERR_RET(writePixelsStart(s_line_buf, line_bytes, false));
    XMC_ERR_RET(writePixelsComplete());
  }
  return XMC_OK;
}

XmcStatus setWindow(int x, int y, int width, int height) {
  int xe = x + width  - 1;
  int ye = y + height - 1;
  writeCommand4Params(CMD_CASET,
                      (uint8_t)(x  >> 8), (uint8_t)(x  & 0xFF),
                      (uint8_t)(xe >> 8), (uint8_t)(xe & 0xFF));
  writeCommand4Params(CMD_RASET,
                      (uint8_t)(y  >> 8), (uint8_t)(y  & 0xFF),
                      (uint8_t)(ye >> 8), (uint8_t)(ye & 0xFF));
  return XMC_OK;
}

XmcStatus writePixelsStart(const void *data, uint32_t num_bytes,
                           bool /*repeated*/) {
  XMC_ERR_RET(begin_command(CMD_RAMWR));
  gpio::write(XMC_PIN_DISPLAY_DC, 1);
  dma::Config cfg = {.ptr = (void *)data, .element_size = 1,
                     .length = (int)num_bytes};
  XmcStatus sts = spi::dmaWriteStart(&cfg, XMC_PIN_DISPLAY_CS);
  if (sts != XMC_OK) end_command();
  return sts;
}

XmcStatus writePixelsComplete() {
  XmcStatus sts = spi::dmaComplete();
  end_command();
  return sts;
}

XmcStatus writeCommand(const uint8_t cmd, const uint8_t *params,
                       uint32_t numParams) {
  XMC_ERR_RET(begin_command(cmd));
  XmcStatus sts = XMC_OK;
  if (numParams > 0) sts = write_data(params, numParams);
  end_command();
  return sts;
}

// ---- Internal helpers -----------------------------------------------------

static XmcStatus begin_command(uint8_t cmd) {
  XMC_ERR_RET(spi::lock());
  s_in_transaction = true;
  spi::setBaudrate(spi::getPreferredFrequency(Chipset::DISPLAY));
  XmcStatus sts = XMC_OK;
  do {
    gpio::write(XMC_PIN_DISPLAY_DC, 0);
    gpio::write(XMC_PIN_DISPLAY_CS, 0);
    XMC_ERR_BRK(sts, spi::writeBlocking(&cmd, 1));
  } while (0);
  if (sts != XMC_OK) end_command();
  return sts;
}

static void end_command() {
  if (!s_in_transaction) return;
  s_in_transaction = false;
  gpio::write(XMC_PIN_DISPLAY_CS, 1);
  gpio::write(XMC_PIN_DISPLAY_DC, 1);
  spi::unlock();
}

static XmcStatus write_data(const uint8_t *data, uint32_t size) {
  gpio::write(XMC_PIN_DISPLAY_DC, 1);
  return spi::writeBlocking(data, size);
}

}  // namespace xmc::display
