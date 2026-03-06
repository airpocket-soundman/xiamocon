#include "xmc/display.h"
#include "xmc/hw/gpio.h"
#include "xmc/hw/pins.h"
#include "xmc/hw/spi.h"
#include "xmc/hw/timer.h"
#include "xmc/ioex.h"

typedef enum {
  XMC_ST7789_NOP = 0x00,
  XMC_ST7789_SOFTWARE_RESET = 0x01,
  XMC_ST7789_READ_DISP_ID = 0x04,
  XMC_ST7789_READ_ERROR_DSI = 0x05,
  XMC_ST7789_READ_DISP_STATUS = 0x09,
  XMC_ST7789_READ_DISP_POWER_MODE = 0x0A,
  XMC_ST7789_READ_DISP_MADCTRL = 0x0B,
  XMC_ST7789_READ_DISP_PIXEL_FORMAT = 0x0C,
  XMC_ST7789_READ_DISP_IMAGE_MODE = 0x0D,
  XMC_ST7789_READ_DISP_SIGNAL_MODE = 0x0E,
  XMC_ST7789_READ_DISP_SELF_DIAGNOSTIC = 0x0F,
  XMC_ST7789_ENTER_SLEEP_MODE = 0x10,
  XMC_ST7789_SLEEP_OUT = 0x11,
  XMC_ST7789_PARTIAL_MODE_ON = 0x12,
  XMC_ST7789_NORMAL_DISP_MODE_ON = 0x13,
  XMC_ST7789_DISP_INVERSION_OFF = 0x20,
  XMC_ST7789_DISP_INVERSION_ON = 0x21,
  XMC_ST7789_PIXEL_OFF = 0x22,
  XMC_ST7789_PIXEL_ON = 0x23,
  XMC_ST7789_DISPLAY_OFF = 0x28,
  XMC_ST7789_DISPLAY_ON = 0x29,
  XMC_ST7789_COLUMN_ADDRESS_SET = 0x2A,
  XMC_ST7789_PAGE_ADDRESS_SET = 0x2B,
  XMC_ST7789_MEMORY_WRITE = 0x2C,
  XMC_ST7789_MEMORY_READ = 0x2E,
  XMC_ST7789_PARTIAL_AREA = 0x30,
  XMC_ST7789_VERT_SCROLL_DEFINITION = 0x33,
  XMC_ST7789_TEARING_EFFECT_LINE_OFF = 0x34,
  XMC_ST7789_TEARING_EFFECT_LINE_ON = 0x35,
  XMC_ST7789_MEMORY_ACCESS_CONTROL = 0x36,
  XMC_ST7789_VERT_SCROLL_START_ADDRESS = 0x37,
  XMC_ST7789_IDLE_MODE_OFF = 0x38,
  XMC_ST7789_IDLE_MODE_ON = 0x39,
  XMC_ST7789_INTERFACE_PIXEL_FORMAT = 0x3A,
  XMC_ST7789_MEMORY_WRITE_CONTINUE = 0x3C,
  XMC_ST7789_MEMORY_READ_CONTINUE = 0x3E,
  XMC_ST7789_SET_TEAR_SCANLINE = 0x44,
  XMC_ST7789_GET_SCANLINE = 0x45,
  XMC_ST7789_WRITE_DISPLAY_BRIGHTNESS = 0x51,
  XMC_ST7789_READ_DISPLAY_BRIGHTNESS = 0x52,
  XMC_ST7789_WRITE_CTRL_DISPLAY = 0x53,
  XMC_ST7789_READ_CTRL_DISPLAY = 0x54,
  XMC_ST7789_WRITE_CONTENT_ADAPT_BRIGHTNESS = 0x55,
  XMC_ST7789_READ_CONTENT_ADAPT_BRIGHTNESS = 0x56,
  XMC_ST7789_WRITE_MIN_CAB_LEVEL = 0x5E,
  XMC_ST7789_READ_MIN_CAB_LEVEL = 0x5F,
  XMC_ST7789_READ_ABC_SELF_DIAG_RES = 0x68,
  XMC_ST7789_READ_ID1 = 0xDA,
  XMC_ST7789_READ_ID2 = 0xDB,
  XMC_ST7789_READ_ID3 = 0xDC,
  XMC_ST7789_INTERFACE_MODE_CONTROL = 0xB0,
  XMC_ST7789_FRAME_RATE_CONTROL_NORMAL = 0xB1,
  XMC_ST7789_FRAME_RATE_CONTROL_IDLE_8COLOR = 0xB2,
  XMC_ST7789_FRAME_RATE_CONTROL_PARTIAL = 0xB3,
  XMC_ST7789_DISPLAY_INVERSION_CONTROL = 0xB4,
  XMC_ST7789_BLANKING_PORCH_CONTROL = 0xB5,
  XMC_ST7789_DISPLAY_FUNCTION_CONTROL = 0xB6,
  XMC_ST7789_ENTRY_MODE_SET = 0xB7,
  XMC_ST7789_BACKLIGHT_CONTROL_1 = 0xB9,
  XMC_ST7789_BACKLIGHT_CONTROL_2 = 0xBA,
  XMC_ST7789_HS_LANES_CONTROL = 0xBE,
  XMC_ST7789_POWER_CONTROL_1 = 0xC0,
  XMC_ST7789_POWER_CONTROL_2 = 0xC1,
  XMC_ST7789_POWER_CONTROL_NORMAL_3 = 0xC2,
  XMC_ST7789_POWER_CONTROL_IDEL_4 = 0xC3,
  XMC_ST7789_POWER_CONTROL_PARTIAL_5 = 0xC4,
  XMC_ST7789_VCOM_CONTROL_1 = 0xC5,
  XMC_ST7789_CABC_CONTROL_1 = 0xC6,
  XMC_ST7789_CABC_CONTROL_2 = 0xC8,
  XMC_ST7789_CABC_CONTROL_3 = 0xC9,
  XMC_ST7789_CABC_CONTROL_4 = 0xCA,
  XMC_ST7789_CABC_CONTROL_5 = 0xCB,
  XMC_ST7789_CABC_CONTROL_6 = 0xCC,
  XMC_ST7789_CABC_CONTROL_7 = 0xCD,
  XMC_ST7789_CABC_CONTROL_8 = 0xCE,
  XMC_ST7789_CABC_CONTROL_9 = 0xCF,
  XMC_ST7789_NVMEM_WRITE = 0xD0,
  XMC_ST7789_NVMEM_PROTECTION_KEY = 0xD1,
  XMC_ST7789_NVMEM_STATUS_READ = 0xD2,
  XMC_ST7789_READ_ID4 = 0xD3,
  XMC_ST7789_ADJUST_CONTROL_1 = 0xD7,
  XMC_ST7789_READ_ID_VERSION = 0xD8,
  XMC_ST7789_POSITIVE_GAMMA_CORRECTION = 0xE0,
  XMC_ST7789_NEGATIVE_GAMMA_CORRECTION = 0xE1,
  XMC_ST7789_DIGITAL_GAMMA_CONTROL_1 = 0xE2,
  XMC_ST7789_DIGITAL_GAMMA_CONTROL_2 = 0xE3,
  XMC_ST7789_SET_IMAGE_FUNCTION = 0xE9,
  XMC_ST7789_ADJUST_CONTROL_2 = 0xF2,
  XMC_ST7789_ADJUST_CONTROL_3 = 0xF7,
  XMC_ST7789_ADJUST_CONTROL_4 = 0xF8,
  XMC_ST7789_ADJUST_CONTROL_5 = 0xF9,
  XMC_ST7789_SPI_READ_COMMAND_SETTING = 0xFB,
  XMC_ST7789_ADJUST_CONTROL_6 = 0xFC,
  XMC_ST7789_ADJUST_CONTROL_7 = 0xFF,
} xmc_st7789_command_t;

static xmc_display_intf_format_t current_format = XMC_DISP_INTF_FORMAT_RGB444;
static uint8_t
    line_buffer[XMC_DISPLAY_WIDTH * 3];  // max 3 bytes per pixel for RGB666

static xmc_status_t begin_command(uint8_t cmd);
static void end_command();
static xmc_status_t write_data(const uint8_t *data, uint32_t size);

xmc_status_t xmc_display_init(xmc_display_intf_format_t format, int rotation) {
  current_format = format;

  xmc_gpio_set_dir(XMC_PIN_DISPLAY_CS, true);
  xmc_gpio_set_dir(XMC_PIN_DISPLAY_DC, true);
  xmc_gpio_write(XMC_PIN_DISPLAY_CS, 1);
  xmc_gpio_write(XMC_PIN_DISPLAY_DC, 1);

  XMC_ERR_RET(xmc_ioex_set_dir(XMC_IOEX_PIN_DISPLAY_RESET, true));
  XMC_ERR_RET(xmc_ioex_write(XMC_IOEX_PIN_DISPLAY_RESET, 0));
  xmc_sleep_ms(100);
  XMC_ERR_RET(xmc_ioex_write(XMC_IOEX_PIN_DISPLAY_RESET, 1));
  xmc_sleep_ms(100);

  XMC_ERR_RET(xmc_display_write_command_0p(XMC_ST7789_SOFTWARE_RESET));
  xmc_sleep_ms(200);

  XMC_ERR_RET(xmc_display_write_command_0p(XMC_ST7789_SLEEP_OUT));
  xmc_sleep_ms(200);

  uint8_t intf_format;
  switch (current_format) {
    case XMC_DISP_INTF_FORMAT_RGB444: intf_format = 0x53; break;
    default: return XMC_ERR_DISPLAY_UNSUPPORTED_FORMAT;
  }
  XMC_ERR_RET(xmc_display_write_command_1p(XMC_ST7789_INTERFACE_PIXEL_FORMAT,
                                           intf_format));

  // todo: support rotation
  // #if 0
  //     writeCommand(Command::MEMORY_ACCESS_CONTROL, 0x48);
  // #else
  //     // writeCommand(Command::MEMORY_ACCESS_CONTROL, 0xE8);
  //     writeCommand(Command::MEMORY_ACCESS_CONTROL, 0x28);
  // #endif

  XMC_ERR_RET(xmc_display_write_command_0p(XMC_ST7789_DISP_INVERSION_ON));

  XMC_ERR_RET(xmc_display_fill_rect(0, 0, XMC_DISPLAY_WIDTH, XMC_DISPLAY_HEIGHT,
                                    0x0000));

  XMC_ERR_RET(xmc_display_write_command_0p(XMC_ST7789_DISPLAY_ON));
  xmc_sleep_ms(25);
  return XMC_OK;
}

xmc_status_t xmc_display_deinit() {
  xmc_gpio_set_pullup(XMC_PIN_DISPLAY_CS, true);
  xmc_gpio_set_pullup(XMC_PIN_DISPLAY_DC, true);
  xmc_gpio_set_dir(XMC_PIN_DISPLAY_CS, false);
  xmc_gpio_set_dir(XMC_PIN_DISPLAY_DC, false);
  XMC_ERR_RET(xmc_ioex_write(XMC_IOEX_PIN_DISPLAY_RESET, 0));
  return XMC_OK;
}

xmc_status_t xmc_display_clear(uint32_t color) {
  XMC_ERR_RET(xmc_display_fill_rect(0, 0, XMC_DISPLAY_WIDTH, XMC_DISPLAY_HEIGHT,
                                    color));
  return XMC_OK;
}

xmc_status_t xmc_display_fill_rect(int x, int y, int width, int height,
                                   uint32_t color) {

  if (x < 0) {
    width += x;
    x = 0;
    if (width <= 0) return XMC_OK;
  }
  if (x + width > XMC_DISPLAY_WIDTH) {
    width = XMC_DISPLAY_WIDTH - x;
    if (width <= 0) return XMC_OK;
  }
  if (y < 0) {
    height += y;
    y = 0;
    if (height <= 0) return XMC_OK;
  }
  if (y + height > XMC_DISPLAY_HEIGHT) {
    height = XMC_DISPLAY_HEIGHT - y;
    if (height <= 0) return XMC_OK;
  }

  int line_bytes;
  switch (current_format) {
    case XMC_DISP_INTF_FORMAT_RGB444:
      line_bytes = width * 3  / 2;
      for (int i = 0; i < line_bytes; i += 3) {
        line_buffer[i + 0] = (color >> 4) & 0xFF;
        line_buffer[i + 1] = ((color & 0x0F) << 4) | ((color >> 8) & 0x0F);
        line_buffer[i + 2] = color & 0xFF;
      }
      break;
    default: return XMC_ERR_DISPLAY_UNSUPPORTED_FORMAT;
  }

  for (int i = 0; i < height; i++) {
    XMC_ERR_RET(xmc_display_set_window(x, y + i, width, 1));
    XMC_ERR_RET(xmc_display_write_pixels_start(line_buffer, line_bytes, false));
    XMC_ERR_RET(xmc_display_write_pixels_complete());
  }
  return XMC_OK;
}

xmc_status_t xmc_display_set_window(int x, int y, int width, int height) {
  int x_end = x + width - 1;
  int y_end = y + height - 1;
  XMC_ERR_RET(xmc_display_write_command_4p(XMC_ST7789_COLUMN_ADDRESS_SET,
                                           x >> 8, x & 0xFF, x_end >> 8,
                                           x_end & 0xFF));
  XMC_ERR_RET(xmc_display_write_command_4p(XMC_ST7789_PAGE_ADDRESS_SET, y >> 8,
                                           y & 0xFF, y_end >> 8, y_end & 0xFF));
  return XMC_OK;
}

xmc_status_t xmc_display_write_pixels_start(const void *data,
                                            uint32_t num_bytes, bool repeated) {
  XMC_ERR_RET(begin_command(XMC_ST7789_MEMORY_WRITE));
  xmc_gpio_write(XMC_PIN_DISPLAY_DC, 1);
  xmc_dma_config_t cfg = {
      .ptr = (void *)data,
      .element_size = 1,
      .length = num_bytes,
      .increment = !repeated,
  };
  xmc_status_t sts = xmc_spi_dma_write_start(&cfg, XMC_PIN_DISPLAY_CS);
  if (sts != XMC_OK) {
    end_command();
  }
  return sts;
}

xmc_status_t xmc_display_write_pixels_complete() {
  return xmc_spi_dma_complete();
}

xmc_status_t xmc_display_write_command(const uint8_t cmd, const uint8_t *params,
                                       uint32_t num_params) {
  XMC_ERR_RET(begin_command(cmd));
  xmc_status_t sts = XMC_OK;
  if (num_params > 0) {
    sts = write_data(params, num_params);
  }
  end_command();
  return sts;
}

static xmc_status_t begin_command(uint8_t cmd) {
  XMC_ERR_RET(xmc_spi_begin_transaction());
  xmc_spi_set_baudrate(xmc_spi_get_preferred_frequency(XMC_SPI_DEV_DISPLAY));
  xmc_status_t sts = XMC_OK;
  do {
    xmc_gpio_write(XMC_PIN_DISPLAY_DC, 0);
    xmc_gpio_write(XMC_PIN_DISPLAY_CS, 0);
    XMC_ERR_BRK(sts, xmc_spi_write_blocking(&cmd, 1));
  } while (0);
  if (sts != XMC_OK) {
    end_command();
  }
  return sts;
}

static void end_command() {
  xmc_gpio_write(XMC_PIN_DISPLAY_CS, 1);
  xmc_gpio_write(XMC_PIN_DISPLAY_DC, 1);
  xmc_spi_end_transaction();
}

static xmc_status_t write_data(const uint8_t *data, uint32_t size) {
  xmc_gpio_write(XMC_PIN_DISPLAY_DC, 1);
  return xmc_spi_write_blocking(data, size);
}
