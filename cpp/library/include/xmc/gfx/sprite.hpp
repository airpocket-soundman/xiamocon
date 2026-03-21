#ifndef XMC_SPRITE_HPP
#define XMC_SPRITE_HPP

#include "gfxfont.h"
#include "xmc/display.h"
#include "xmc/geo.hpp"
#include "xmc/gfx/gfx_common.hpp"
#include "xmc/hw/ram.h"

#include <memory>

namespace xmc {

class SpriteClass;
using Sprite = std::shared_ptr<SpriteClass>;

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
class SpriteClass {
 protected:
  pixel_format_t format_;
  int width_;
  int height_;
  uint32_t stride_;
  void *data_;
  bool auto_free_;

  const GFXfont *font = nullptr;
  int font_size = 1;
  uint16_t text_color = 0;
  int cursor_x = 0;
  int cursor_y = 0;

 public:
  SpriteClass(pixel_format_t format, int width, int height, uint32_t stride,
              void *data, bool auto_free)
      : format_(format),
        width_(width),
        height_(height),
        stride_(stride),
        data_(data),
        auto_free_(auto_free) {}

  ~SpriteClass() {
    if (auto_free_ && data_) {
      xmc_free(data_);
      data_ = nullptr;
    }
  }

  inline pixel_format_t format() const { return format_; }
  inline int width() const { return width_; }
  inline int height() const { return height_; }
  inline uint32_t stride() const { return stride_; }

  inline void *line_ptr(int y) const {
    if (y < 0 || y >= height_) return nullptr;
    return (uint8_t *)data_ + stride_ * y;
  }

  void set_pixel(int x, int y, raw_color color) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return;
    on_set_pixel(x, y, color);
  }
  raw_color get_pixel(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return 0;
    return on_get_pixel(x, y);
  }

  void set_font(const GFXfont *font, int size = 1) {
    this->font = font;
    this->font_size = size;
  }
  void set_cursor(int x, int y) {
    this->cursor_x = x;
    this->cursor_y = y;
  }
  void set_text_color(raw_color color) { this->text_color = color; }

  void draw_string(const char *str) {
    if (!font || !str) return;
    int x = cursor_x;
    for (const char *p = str; *p; p++) {
      if (*p == '\n') {
        x = cursor_x;
        cursor_y += font->yAdvance * font_size;
      } else {
        x += on_draw_char(x, cursor_y, *p);
      }
    }
    cursor_x = x;
  }

  xmc_status_t start_transfer_to_display(int dx, int dy) {
    return on_start_transfer_to_display(dx, dy, 0, height_);
  }

  xmc_status_t complete_transfer() {
    return xmc_display_write_pixels_complete();
  }

  void clear(raw_color color) { on_fill_rect(0, 0, width_, height_, color); }

  void fill_rect(int x, int y, int w, int h, raw_color color) {
    if (w < 0) {
      x += w;
      w = -w;
    }
    if (h < 0) {
      y += h;
      h = -h;
    }
    clip_rect(&x, &y, &w, &h, width_, height_);
    if (w <= 0 || h <= 0) return;
    on_fill_rect(x, y, w, h, color);
  }

  void draw_rect(int x, int y, int w, int h, raw_color color) {
    if (w < 0) {
      x += w;
      w = -w;
    }
    if (h < 0) {
      y += h;
      h = -h;
    }
    fill_rect(x, y, w, 1, color);
    fill_rect(x, y + h, w, 1, color);
    fill_rect(x, y, 1, h - 1, color);
    fill_rect(x + w, y, 1, h - 1, color);
  }

  // todo: delete
  void fill_triangle(const vec3 *verts, const int *indices, int offset,
                     raw_color color) {
    int i0 = indices[offset + 0];
    int i1 = indices[offset + 1];
    int i2 = indices[offset + 2];

    // 反時計回りのときは描画しない
    float ax = verts[i1].x - verts[i0].x;
    float ay = verts[i1].y - verts[i0].y;
    float bx = verts[i2].x - verts[i0].x;
    float by = verts[i2].y - verts[i0].y;
    if (ax * by - ay * bx <= 0) return;

    if (verts[i0].y > verts[i1].y) {
      int t = i0;
      i0 = i1;
      i1 = t;
    }
    if (verts[i1].y > verts[i2].y) {
      int t = i1;
      i1 = i2;
      i2 = t;
    }
    if (verts[i0].y > verts[i1].y) {
      int t = i0;
      i0 = i1;
      i1 = t;
    }

    float x0 = verts[i0].x, y0 = verts[i0].y;
    float x1 = verts[i1].x, y1 = verts[i1].y;
    float x2 = verts[i2].x, y2 = verts[i2].y;

    int iy_min = (int)ceilf(y0);
    int iy_max = (int)floorf(y2);
    if (iy_min < 0) iy_min = 0;
    if (iy_max >= height_) iy_max = height_ - 1;

    for (int iy = iy_min; iy <= iy_max; iy++) {
      float y = (float)iy;
      float xa = (y2 - y0) > 0.0f ? x0 + (x2 - x0) * (y - y0) / (y2 - y0) : x0;
      float xb;
      if (y < y1) {
        xb = (y1 - y0) > 0.0f ? x0 + (x1 - x0) * (y - y0) / (y1 - y0) : x0;
      } else {
        xb = (y2 - y1) > 0.0f ? x1 + (x2 - x1) * (y - y1) / (y2 - y1) : x1;
      }
      if (xa > xb) {
        float t = xa;
        xa = xb;
        xb = t;
      }
      int ix_min = (int)ceilf(xa);
      int ix_max = (int)floorf(xb);
      if (ix_min < 0) ix_min = 0;
      if (ix_max >= width_) ix_max = width_ - 1;
      on_fill_rect(ix_min, iy, ix_max - ix_min + 1, 1, color);
    }
  }

  inline void draw_image(const Sprite &image, int dx, int dy, int w, int h,
                         int sx, int sy) {
    rect_t view = {0, 0, width_, height_};
    rect_t dst = {dx, dy, w, h};
    dst = dst.intersect(view);
    if (dst.width <= 0 || dst.height <= 0) return;
    sx += dst.x - dx;
    sy += dst.y - dy;
    dx = dst.x;
    dy = dst.y;
    w = dst.width;
    h = dst.height;
    rect_t img = {0, 0, image->width(), image->height()};
    rect_t src = {sx, sy, w, h};
    src = src.intersect(img);
    if (src.width <= 0 || src.height <= 0) return;
    dx += src.x - sx;
    dy += src.y - sy;
    w = src.width;
    h = src.height;
    on_draw_image(image, dx, dy, w, h, sx, sy);
  }

 protected:
  virtual void on_set_pixel(int x, int y, raw_color color) = 0;
  virtual raw_color on_get_pixel(int x, int y) const = 0;
  virtual void on_fill_rect(int x, int y, int w, int h, raw_color color) = 0;
  virtual void on_draw_image(const Sprite &image, int dx, int dy, int w, int h,
                             int sx, int sy) = 0;
  virtual xmc_status_t on_start_transfer_to_display(int dx, int dy, int sy,
                                                    int h) = 0;
  virtual int on_draw_char(int x, int y, char c) {
    if (!font) return 0;
    if (c < font->first || c > font->last) return 0;
    GFXglyph *glyph = &font->glyph[c - font->first];
    int x0 = x + glyph->xOffset * font_size;
    int y0 = y + glyph->yOffset * font_size;
    int w = glyph->width * font_size;
    int h = glyph->height * font_size;
    uint8_t *bitmap = font->bitmap + glyph->bitmapOffset;
    uint8_t byte = 0;
    int ibit = 0;
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        if (ibit == 0) {
          byte = *bitmap++;
        }
        if (byte & 0x80) {
          if (font_size == 1) {
            set_pixel(x0 + i, y0 + j, text_color);
          } else {
            fill_rect(x0 + i * font_size, y0 + j * font_size, font_size,
                      font_size, text_color);
          }
        }
        byte <<= 1;
        ibit = (ibit + 1) % 8;
      }
    }
    return glyph->xAdvance * font_size;
  }
};

}  // namespace xmc

#endif