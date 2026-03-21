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
 public:
  const pixel_format_t format;
  const int width;
  const int height;
  const uint32_t stride;
  void *data;
  const bool autoFree;

 protected:
  const GFXfont *font = nullptr;
  int fontSize = 1;
  uint16_t textColor = 0;
  int cursorX = 0;
  int cursorY = 0;

 public:
  SpriteClass(pixel_format_t format, int width, int height, uint32_t stride,
              void *data, bool auto_free)
      : format(format),
        width(width),
        height(height),
        stride(stride),
        data(data),
        autoFree(auto_free) {}

  ~SpriteClass() {
    if (autoFree && data) {
      xmcFree(data);
      data = nullptr;
    }
  }

  inline void *linePtr(int y) const {
    if (y < 0 || y >= height) return nullptr;
    return (uint8_t *)data + stride * y;
  }

  void set_pixel(int x, int y, raw_color color) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    onSetPixel(x, y, color);
  }
  raw_color get_pixel(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return 0;
    return onGetPixel(x, y);
  }

  void set_font(const GFXfont *font, int size = 1) {
    this->font = font;
    this->fontSize = size;
  }
  void set_cursor(int x, int y) {
    this->cursorX = x;
    this->cursorY = y;
  }
  void set_text_color(raw_color color) { this->textColor = color; }

  void draw_string(const char *str) {
    if (!font || !str) return;
    int x = cursorX;
    for (const char *p = str; *p; p++) {
      if (*p == '\n') {
        x = cursorX;
        cursorY += font->yAdvance * fontSize;
      } else {
        x += on_draw_char(x, cursorY, *p);
      }
    }
    cursorX = x;
  }

  XmcStatus start_transfer_to_display(int dx, int dy) {
    return on_start_transfer_to_display(dx, dy, 0, height);
  }

  XmcStatus complete_transfer() {
    return xmc_displayWritePixelsComplete();
  }

  void clear(raw_color color) { onFillRect(0, 0, width, height, color); }

  void fill_rect(int x, int y, int w, int h, raw_color color) {
    if (w < 0) {
      x += w;
      w = -w;
    }
    if (h < 0) {
      y += h;
      h = -h;
    }
    clipRect(&x, &y, &w, &h, width, height);
    if (w <= 0 || h <= 0) return;
    onFillRect(x, y, w, h, color);
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
    if (iy_max >= height) iy_max = height - 1;

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
      if (ix_max >= width) ix_max = width - 1;
      onFillRect(ix_min, iy, ix_max - ix_min + 1, 1, color);
    }
  }

  inline void draw_image(const Sprite &image, int dx, int dy, int w, int h,
                         int sx, int sy) {
    rect_t view = {0, 0, width, height};
    rect_t dst = {dx, dy, w, h};
    dst = dst.intersect(view);
    if (dst.width <= 0 || dst.height <= 0) return;
    sx += dst.x - dx;
    sy += dst.y - dy;
    dx = dst.x;
    dy = dst.y;
    w = dst.width;
    h = dst.height;
    rect_t img = {0, 0, image->width, image->height};
    rect_t src = {sx, sy, w, h};
    src = src.intersect(img);
    if (src.width <= 0 || src.height <= 0) return;
    dx += src.x - sx;
    dy += src.y - sy;
    w = src.width;
    h = src.height;
    onDrawImage(image, dx, dy, w, h, sx, sy);
  }

 protected:
  virtual void onSetPixel(int x, int y, raw_color color) = 0;
  virtual raw_color onGetPixel(int x, int y) const = 0;
  virtual void onFillRect(int x, int y, int w, int h, raw_color color) = 0;
  virtual void onDrawImage(const Sprite &image, int dx, int dy, int w, int h,
                             int sx, int sy) = 0;
  virtual XmcStatus on_start_transfer_to_display(int dx, int dy, int sy,
                                                    int h) = 0;
  virtual int on_draw_char(int x, int y, char c) {
    if (!font) return 0;
    if (c < font->first || c > font->last) return 0;
    GFXglyph *glyph = &font->glyph[c - font->first];
    int x0 = x + glyph->xOffset * fontSize;
    int y0 = y + glyph->yOffset * fontSize;
    int w = glyph->width * fontSize;
    int h = glyph->height * fontSize;
    uint8_t *bitmap = font->bitmap + glyph->bitmapOffset;
    uint8_t byte = 0;
    int ibit = 0;
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        if (ibit == 0) {
          byte = *bitmap++;
        }
        if (byte & 0x80) {
          if (fontSize == 1) {
            set_pixel(x0 + i, y0 + j, textColor);
          } else {
            fill_rect(x0 + i * fontSize, y0 + j * fontSize, fontSize, fontSize,
                      textColor);
          }
        }
        byte <<= 1;
        ibit = (ibit + 1) % 8;
      }
    }
    return glyph->xAdvance * fontSize;
  }
};

}  // namespace xmc

#endif