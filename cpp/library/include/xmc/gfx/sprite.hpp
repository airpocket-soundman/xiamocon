#ifndef XMC_SPRITE_HPP
#define XMC_SPRITE_HPP

#include "xmc/display.h"
#include "xmc/geo/geo_common.hpp"
#include "xmc/gfx/gfx_common.hpp"
#include "xmc/hw/ram.h"

namespace xmc {

/**
 * Base class for sprites. This is a template class that takes a pixel format
 * and a pixel type as template parameters. The pixel format is used to
 * determine how the pixel data is stored. The pixel type is used to represent
 * the color of a pixel in the sprite. The Sprite class provides basic
 * functionality for managing a sprite, such as setting and getting pixel
 * values, filling rectangles, and starting a transfer to the display. The
 * actual implementation of these functions is left to derived classes that
 * specialize the template for specific pixel formats and types.
 */
template <pixel_format_t prm_PIXEL_FORMAT, typename pixel_t>
class Sprite {
 public:
  static constexpr pixel_format_t FORMAT = prm_PIXEL_FORMAT;
  const int width;
  const int height;
  const uint32_t stride;
  void *data;
  const bool auto_free;

  Sprite(int width, int height, uint32_t stride, void *data, bool auto_free)
      : width(width),
        height(height),
        stride(stride),
        data(data),
        auto_free(auto_free) {}

  ~Sprite() {
    if (auto_free && data) {
      xmc_free(data);
      data = nullptr;
    }
  }

  Sprite(const Sprite &) = delete;
  Sprite &operator=(const Sprite &) = delete;

  Sprite(Sprite &&other) noexcept
      : width(other.width),
        height(other.height),
        stride(other.stride),
        data(other.data),
        auto_free(other.auto_free) {
    other.data = nullptr;
  }

  Sprite &operator=(Sprite &&other) noexcept {
    if (this != &other) {
      if (auto_free && data) {
        xmc_free(data);
      }
      width = other.width;
      height = other.height;
      stride = other.stride;
      data = other.data;
      auto_free = other.auto_free;
      other.data = nullptr;
    }
    return *this;
  }

  inline void *line_ptr(int y) const {
    if (y < 0 || y >= height) return nullptr;
    return (uint8_t *)data + stride * y;
  }

  virtual void set_pixel(int x, int y, pixel_t color) = 0;
  virtual pixel_t get_pixel(int x, int y) const = 0;

  virtual void on_fill_rect(int x, int y, int w, int h, pixel_t color) = 0;

  virtual xmc_status_t start_transfer_to_display(int x, int y) = 0;

  xmc_status_t complete_transfer() {
    return xmc_display_write_pixels_complete();
  }

  void fill_rect(int x, int y, int w, int h, pixel_t color) {
    clip_rect(&x, &y, &w, &h, width, height);
    if (w <= 0 || h <= 0) return;
    on_fill_rect(x, y, w, h, color);
  }

  void clear(pixel_t color) { fill_rect(0, 0, width, height, color); }

  void draw_rect(int x, int y, int w, int h, pixel_t color) {
    fill_rect(x, y, w, 1, color);
    fill_rect(x, y + h, w, 1, color);
    fill_rect(x, y, 1, h - 1, color);
    fill_rect(x + w, y, 1, h - 1, color);
  }
};

class Sprite444;
class Sprite4444;

}  // namespace xmc

#endif