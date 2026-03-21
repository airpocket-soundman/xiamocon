#include "xmc/gfx/rasterizer.hpp"

#include <string.h>

namespace xmc {

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
    render_primitive(prim);
  }
}

void RasterizerClass::render_primitive(const Primitive &prim) {
  const Vec3Buffer &prim_pos = prim->position;
  const Vec3Buffer &prim_norm = prim->normal;
  const ColorBuffer &prim_col = prim->color;
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

        // transform
        norm += pos;
        norm = tfx_matrix.transform(norm);
        pos = tfx_matrix.transform(pos);
        norm -= pos;
        norm = norm.normalized();

        // shading
        baked_vertex_t &out = baked_verts[j];
        out.pos = projection_matrix.transform(pos);
        colorf vert_color =
            col * (env_light + parallel_light_color *
                                   fmaxf(0, norm.dot(parallel_light_dir)));
        out.color = vert_color;
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
          render_triangle(baked_verts[0], baked_verts[2], baked_verts[1]);
        } else {
          render_triangle(baked_verts[0], baked_verts[1], baked_verts[2]);
        }
        break;
    }
  }
}

void RasterizerClass::render_triangle(const baked_vertex_t &v0,
                                      const baked_vertex_t &v1,
                                      const baked_vertex_t &v2) {
  const baked_vertex_t *tri[] = {&v0, &v1, &v2};

  int i0 = 0;
  int i1 = 1;
  int i2 = 2;

  // back-face culling
  float ax = tri[i1]->pos.x - tri[i0]->pos.x;
  float ay = tri[i1]->pos.y - tri[i0]->pos.y;
  float bx = tri[i2]->pos.x - tri[i0]->pos.x;
  float by = tri[i2]->pos.y - tri[i0]->pos.y;
  if (ax * by - ay * bx <= 0) return;

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
  float x0 = tri[i0]->pos.x, y0 = tri[i0]->pos.y, z0 = tri[i0]->pos.z;
  float x1 = tri[i1]->pos.x, y1 = tri[i1]->pos.y, z1 = tri[i1]->pos.z;
  float x2 = tri[i2]->pos.x, y2 = tri[i2]->pos.y, z2 = tri[i2]->pos.z;
  float y0to1inv = (y1 - y0) > 1e-8f ? 1.0f / (y1 - y0) : 0.0f;
  float y1to2inv = (y2 - y1) > 1e-8f ? 1.0f / (y2 - y1) : 0.0f;
  float y0to2inv = (y2 - y0) > 1e-8f ? 1.0f / (y2 - y0) : 0.0f;

  float d0 = 255.0f * (tri[i0]->pos.z - z_near) / (z_far - z_near);
  float d1 = 255.0f * (tri[i1]->pos.z - z_near) / (z_far - z_near);
  float d2 = 255.0f * (tri[i2]->pos.z - z_near) / (z_far - z_near);
  float d0to1inv = (d1 - d0) > 1e-8f ? 1.0f / (d1 - d0) : 0.0f;
  float d1to2inv = (d2 - d1) > 1e-8f ? 1.0f / (d2 - d1) : 0.0f;
  float d0to2inv = (d2 - d0) > 1e-8f ? 1.0f / (d2 - d0) : 0.0f;

  colorf c0 = tri[i0]->color;
  colorf c1 = tri[i1]->color;
  colorf c2 = tri[i2]->color;
  colorf c0to1 = c1 - c0;
  colorf c1to2 = c2 - c1;
  colorf c0to2 = c2 - c0;

  int iy_min = (int)ceilf(y0);
  int iy_max = (int)floorf(y2);
  if (iy_min < viewport.y) iy_min = viewport.y;
  if (iy_max >= viewport.bottom()) iy_max = viewport.bottom() - 1;

  // rasterize the triangle using a scanline algorithm
  // todo: optimize
  for (int iy = iy_min; iy <= iy_max; iy++) {
    float y = (float)iy;
    float xa = x0 + (x2 - x0) * (y - y0) * y0to2inv;
    float xb;
    if (y < y1) {
      xb = x0 + (x1 - x0) * (y - y0) * y0to1inv;
    } else {
      xb = x1 + (x2 - x1) * (y - y1) * y1to2inv;
    }
    if (xa > xb) {
      float t = xa;
      xa = xb;
      xb = t;
    }

    float da = d0 + (d2 - d0) * (y - y0) * y0to2inv;
    float db;
    if (y < y1) {
      db = d0 + (d1 - d0) * (y - y0) * y0to1inv;
    } else {
      db = d1 + (d2 - d1) * (y - y1) * y1to2inv;
    }
    float dstep = 0;
    if (xb > xa) {
      dstep = (db - da) / (xb - xa);
    }

    colorf ca = c0 + c0to2 * (y - y0) * y0to2inv;
    colorf cb;
    if (y < y1) {
      cb = c0 + c0to1 * (y - y0) * y0to1inv;
    } else {
      cb = c1 + c1to2 * (y - y1) * y1to2inv;
    }
    colorf cstep = {0, 0, 0, 0};
    if (xb > xa) {
      cstep = (cb - ca) / (xb - xa);
    }

    int ix_min = (int)ceilf(xa);
    int ix_max = (int)floorf(xb);
    if (ix_min < viewport.x) ix_min = viewport.x;
    if (ix_max >= viewport.right()) ix_max = viewport.right() - 1;
    if (target->format() == pixel_format_t::RGB565) {
      uint16_t *cptr = (uint16_t *)target->line_ptr(iy) + ix_min;
      depth_t *dptr = depth_buff + iy * width + ix_min;
      for (int x = ix_min; x <= ix_max; x++) {
        if (0 <= da && da <= 255 && da < *dptr) {
          *dptr = (depth_t)fminf(255, fmaxf(0, da));
          *cptr = ca.to565();
        }
        dptr++;
        cptr++;
        ca += cstep;
        da += dstep;
      }
    }
  }
}

}  // namespace xmc
