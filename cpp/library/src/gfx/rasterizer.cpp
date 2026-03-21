#include "xmc/gfx/rasterizer.hpp"

#include <string.h>

namespace xmc {

static inline void get_uv_mask(int size, uint32_t *mask, uint32_t *shift) {
  // todo: optimize
  if (size >= 256) {
    *mask = 0xFF;
    *shift = 0;
  } else if (size >= 128) {
    *mask = 0x7F;
    *shift = 1;
  } else if (size >= 64) {
    *mask = 0x3F;
    *shift = 2;
  } else if (size >= 32) {
    *mask = 0x1F;
    *shift = 3;
  } else if (size >= 16) {
    *mask = 0x0F;
    *shift = 4;
  } else if (size >= 8) {
    *mask = 0x07;
    *shift = 5;
  } else if (size >= 4) {
    *mask = 0x03;
    *shift = 6;
  } else if (size >= 2) {
    *mask = 0x01;
    *shift = 7;
  } else {
    *mask = 0x00;
    *shift = 8;
  }
}

void RasterizerClass::clear_depth(depth_t value) {
  int pixel_count = width * height;
  if (value == 0 || value == 0xFF) {
    memset(depth_buff, value, sizeof(depth_t) * pixel_count);
  } else {
    for (int i = 0; i < pixel_count; i++) {
      depth_buff[i] = value;
    }
  }
}

void RasterizerClass::render_mesh(const Mesh &mesh) {
  for (const Primitive &prim : mesh->primitives) {
    render_primitive(prim, mesh->material);
  }
}

void RasterizerClass::render_primitive(const Primitive &prim,
                                       const Material &mat) {
  const Vec3Buffer &prim_pos = prim->position;
  const Vec3Buffer &prim_norm = prim->normal;
  const ColorBuffer &prim_col = prim->color;
  const Vec2Buffer &prim_uv = prim->uv;
  const IndexBuffer &prim_idx = prim->indexes;

  if (!prim_pos) return;

  int idx_offset[3];
  int idx_data[3];

  const mat4 &tfx_matrix = matrix_stack[matrix_stack_top];
  int num_verts = prim->num_verts();
  int num_elems = prim->num_elements();

  baked_vertex_t baked_verts[3];

  for (int i = 0; i < num_elems; i++) {
    bool reverse = false;
    uint32_t shift_flags = 0;
    uint32_t fetch_flags = 0;
    switch (prim->mode) {
      case prim_mode_t::POINTS:
        idx_offset[0] = i;
        shift_flags = 0b000;
        fetch_flags = 0b001;
        break;
      case prim_mode_t::LINES:
        idx_offset[0] = i * 2 + 0;
        idx_offset[1] = i * 2 + 1;
        shift_flags = 0b000;
        fetch_flags = 0b011;
        break;
      case prim_mode_t::LINE_LOOP:
      case prim_mode_t::LINE_STRIP:
        idx_offset[0] = i;
        idx_offset[1] = (i + 1) % num_verts;
        if (i == 0) {
          shift_flags = 0b000;
          fetch_flags = 0b011;
        } else {
          shift_flags = 0b001;
          fetch_flags = 0b010;
        }
        break;
      case prim_mode_t::TRIANGLES:
        idx_offset[0] = i * 3 + 0;
        idx_offset[1] = i * 3 + 1;
        idx_offset[2] = i * 3 + 2;
        shift_flags = 0b000;
        fetch_flags = 0b111;
        break;
      case prim_mode_t::TRIANGLE_STRIP:
        idx_offset[0] = i + 0;
        idx_offset[1] = i + 1;
        idx_offset[2] = i + 2;
        if (i == 0) {
          shift_flags = 0b000;
          fetch_flags = 0b111;
        } else {
          shift_flags = 0b011;
          fetch_flags = 0b100;
        }
        reverse = (i % 2 == 1);
        break;
      case prim_mode_t::TRIANGLE_FAN:
        idx_offset[0] = 0;
        idx_offset[1] = i + 1;
        idx_offset[2] = i + 2;
        if (i == 0) {
          shift_flags = 0b000;
          fetch_flags = 0b111;
        } else {
          shift_flags = 0b010;
          fetch_flags = 0b100;
        }
        break;
      default: return;
    }

    if (prim_idx) {
      idx_data[0] = prim_idx->data[idx_offset[0]];
      idx_data[1] = prim_idx->data[idx_offset[1]];
      idx_data[2] = prim_idx->data[idx_offset[2]];
    } else {
      idx_data[0] = idx_offset[0];
      idx_data[1] = idx_offset[1];
      idx_data[2] = idx_offset[2];
    }

    for (int j = 0; j < 2; j++) {
      if (shift_flags & (1 << j)) {
        baked_verts[j] = baked_verts[j + 1];
      }
    }

    for (int j = 0; j < 3; j++) {
      if (fetch_flags & (1 << j)) {
        // fetch attributes
        vec3 pos = prim_pos->data[idx_data[j]];
        vec3 norm = prim_norm ? prim_norm->data[idx_data[j]] : vec3(0, 0, 1);
        colorf col = prim_col ? prim_col->data[idx_data[j]]
                              : colorf(1.0f, 1.0f, 1.0f, 1.0f);
        vec2 uv = prim_uv ? prim_uv->data[idx_data[j]] : vec2(0, 0);

        // transform
        norm += pos;
        norm = tfx_matrix.transform(norm);
        pos = tfx_matrix.transform(pos);
        norm -= pos;
        norm = norm.normalized();

        // shading
        baked_vertex_t &out = baked_verts[j];
        out.pos = projection_matrix.transform(pos);
        colorf vert_color = col;
        if (mat) {
          vert_color *= mat->base_color;
        }
        vert_color *= (env_light + parallel_light_color *
                                       fmaxf(0, norm.dot(parallel_light_dir)));
        out.color = vert_color;
        out.uv = uv;
      }
    }

    switch (prim->mode) {
      case prim_mode_t::POINTS:
        // todo: implement
        break;
      case prim_mode_t::LINES:
      case prim_mode_t::LINE_LOOP:
      case prim_mode_t::LINE_STRIP:
        // todo: implement
        break;
      case prim_mode_t::TRIANGLES:
      case prim_mode_t::TRIANGLE_STRIP:
      case prim_mode_t::TRIANGLE_FAN:
        if (reverse) {
          render_triangle(baked_verts[0], baked_verts[2], baked_verts[1], mat);
        } else {
          render_triangle(baked_verts[0], baked_verts[1], baked_verts[2], mat);
        }
        break;
    }
  }
}

void RasterizerClass::render_triangle(const baked_vertex_t &v0,
                                      const baked_vertex_t &v1,
                                      const baked_vertex_t &v2,
                                      const Material &mat) {
  const baked_vertex_t *tri[] = {&v0, &v1, &v2};

  int i0 = 0;
  int i1 = 1;
  int i2 = 2;

  // back-face culling
  float ax = tri[i1]->pos.x - tri[i0]->pos.x;
  float ay = tri[i1]->pos.y - tri[i0]->pos.y;
  float bx = tri[i2]->pos.x - tri[i0]->pos.x;
  float by = tri[i2]->pos.y - tri[i0]->pos.y;
  if (ax * by - ay * bx >= 0) return;

  // sort vertices by y-coordinate
  if (tri[i0]->pos.y > tri[i1]->pos.y) {
    int t = i0;
    i0 = i1;
    i1 = t;
  }
  if (tri[i1]->pos.y > tri[i2]->pos.y) {
    int t = i1;
    i1 = i2;
    i2 = t;
  }
  if (tri[i0]->pos.y > tri[i1]->pos.y) {
    int t = i0;
    i0 = i1;
    i1 = t;
  }
  float x0 = tri[i0]->pos.x, y0 = tri[i0]->pos.y;
  float x1 = tri[i1]->pos.x, y1 = tri[i1]->pos.y;
  float x2 = tri[i2]->pos.x, y2 = tri[i2]->pos.y;
  float y0to1inv = (y1 - y0) > 1e-8f ? 1.0f / (y1 - y0) : 0.0f;
  float y1to2inv = (y2 - y1) > 1e-8f ? 1.0f / (y2 - y1) : 0.0f;
  float y0to2inv = (y2 - y0) > 1e-8f ? 1.0f / (y2 - y0) : 0.0f;

  float z0 = 255.0f * (tri[i0]->pos.z - z_near) / (z_far - z_near);
  float z1 = 255.0f * (tri[i1]->pos.z - z_near) / (z_far - z_near);
  float z2 = 255.0f * (tri[i2]->pos.z - z_near) / (z_far - z_near);

  colorf c0 = tri[i0]->color;
  colorf c1 = tri[i1]->color;
  colorf c2 = tri[i2]->color;
  colorf c0to1 = c1 - c0;
  colorf c1to2 = c2 - c1;
  colorf c0to2 = c2 - c0;

  vec2 uv0 = tri[i0]->uv;
  vec2 uv1 = tri[i1]->uv;
  vec2 uv2 = tri[i2]->uv;
  vec2 uv0to1 = uv1 - uv0;
  vec2 uv1to2 = uv2 - uv1;
  vec2 uv0to2 = uv2 - uv0;

  int iy_min = (int)ceilf(y0);
  int iy_max = (int)floorf(y2);
  if (iy_min < viewport.y) iy_min = viewport.y;
  if (iy_max >= viewport.bottom()) iy_max = viewport.bottom() - 1;

  const uint16_t *tex_data = nullptr;
  uint32_t tex_stride = 0;
  uint32_t tex_u_mask = 0, tex_v_mask = 0;
  uint32_t tex_u_shift = 0, tex_v_shift = 0;
  if (mat && mat->color_texture) {
    const auto &tex = mat->color_texture;
    tex_data = (const uint16_t *)tex->line_ptr(0);
    tex_stride = tex->stride() / sizeof(uint16_t);
    get_uv_mask(tex->width(), &tex_u_mask, &tex_u_shift);
    get_uv_mask(tex->height(), &tex_v_mask, &tex_v_shift);
  }

  // rasterize the triangle using a scanline algorithm
  // todo: optimize
  for (int iy = iy_min; iy <= iy_max; iy++) {
    float y = (float)iy;

    float xa = x0 + (x2 - x0) * (y - y0) * y0to2inv;
    float za = z0 + (z2 - z0) * (y - y0) * y0to2inv;
    colorf ca = c0 + c0to2 * (y - y0) * y0to2inv;
    vec2 uva = uv0 + uv0to2 * (y - y0) * y0to2inv;

    float xb;
    float zb;
    colorf cb;
    vec2 uvb;
    if (y < y1) {
      xb = x0 + (x1 - x0) * (y - y0) * y0to1inv;
      zb = z0 + (z1 - z0) * (y - y0) * y0to1inv;
      cb = c0 + c0to1 * (y - y0) * y0to1inv;
      uvb = uv0 + uv0to1 * (y - y0) * y0to1inv;
    } else {
      xb = x1 + (x2 - x1) * (y - y1) * y1to2inv;
      zb = z1 + (z2 - z1) * (y - y1) * y1to2inv;
      cb = c1 + c1to2 * (y - y1) * y1to2inv;
      uvb = uv1 + uv1to2 * (y - y1) * y1to2inv;
    }

    if (xa > xb) {
      std::swap(xa, xb);
      std::swap(za, zb);
      std::swap(ca, cb);
      std::swap(uva, uvb);
    }

    uint32_t ua = (uint32_t)(uva.x * 0x1000000) >> tex_u_shift;
    uint32_t va = (uint32_t)(uva.y * 0x1000000) >> tex_v_shift;
    uint32_t ub = (uint32_t)(uvb.x * 0x1000000) >> tex_u_shift;
    uint32_t vb = (uint32_t)(uvb.y * 0x1000000) >> tex_v_shift;

    float zstep = 0;
    colorf cstep = {0, 0, 0, 0};
    int32_t ustep = 0, vstep = 0;
    if (xb > xa) {
      zstep = (zb - za) / (xb - xa);
      cstep = (cb - ca) / (xb - xa);
      ustep = ((int32_t)ub - (int32_t)ua) / (xb - xa);
      vstep = ((int32_t)vb - (int32_t)va) / (xb - xa);
    }

    int ix_min = (int)ceilf(xa);
    int ix_max = (int)floorf(xb);
    if (ix_min < viewport.x) {
      ix_min = viewport.x;
      za += zstep * (ix_min - xa);
      ca += cstep * (ix_min - xa);
      ua += ustep * (ix_min - xa);
      va += vstep * (ix_min - xa);
    }
    if (ix_max >= viewport.right()) {
      ix_max = viewport.right() - 1;
    }

    if (target->format() == pixel_format_t::RGB565) {
      uint16_t *cptr = (uint16_t *)target->line_ptr(iy) + ix_min;
      depth_t *zptr = depth_buff + iy * width + ix_min;
      for (int x = ix_min; x <= ix_max; x++) {
        if (0 <= za && za <= 255 && za < *zptr) {
          colorf col = ca;
          if (tex_data) {
            uint32_t u = ua >> 16;
            uint32_t v = va >> 16;
            uint16_t texel =
                tex_data[(v & tex_v_mask) * tex_stride + (u & tex_u_mask)];
            col *= colorf::from4444(texel);
          }
          if (col.a >= 0.5f) {
            *cptr = col.to565();
            *zptr = (depth_t)fminf(255, fmaxf(0, za));
          }
        }
        zptr++;
        cptr++;
        za += zstep;
        ca += cstep;
        ua += ustep;
        va += vstep;
      }
    }
  }
}

}  // namespace xmc
