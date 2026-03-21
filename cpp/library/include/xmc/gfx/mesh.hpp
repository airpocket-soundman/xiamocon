#ifndef XMC_GFX_PRIMITIVE_HPP
#define XMC_GFX_PRIMITIVE_HPP

#include "xmc/geo.hpp"
#include "xmc/gfx/colorf.hpp"
#include "xmc/gfx/gfx_common.hpp"
#include "xmc/hw/ram.h"

#include <memory>
#include <vector>

namespace xmc {

template <typename T>
class AttributeBufferClass {
 public:
  T *data;
  int size;

 protected:
  bool auto_free;

 public:
  AttributeBufferClass(T *data, int size, bool auto_free = false)
      : data(data), size(size), auto_free(auto_free) {}

  AttributeBufferClass(int size)
      : data((T *)xmc_malloc(sizeof(T) * size, XMC_RAM_CAP_DMA)),
        size(size),
        auto_free(true) {}

  ~AttributeBufferClass() {
    if (auto_free) {
      if (data) {
        xmc_free((void *)data);
        data = nullptr;
      }
    }
  }
};

using Vec3Buffer = std::shared_ptr<AttributeBufferClass<vec3>>;
static inline Vec3Buffer createVec3Buffer(int size) {
  return std::make_shared<AttributeBufferClass<vec3>>(size);
}
static inline Vec3Buffer createVec3Buffer(vec3 *data, int size,
                                          bool auto_free = false) {
  return std::make_shared<AttributeBufferClass<vec3>>(data, size, auto_free);
}

using ColorBuffer = std::shared_ptr<AttributeBufferClass<colorf>>;
static inline ColorBuffer createColorFBuffer(int size) {
  return std::make_shared<AttributeBufferClass<colorf>>(size);
}
static inline ColorBuffer createColorFBuffer(colorf *data, int size,
                                             bool auto_free = false) {
  return std::make_shared<AttributeBufferClass<colorf>>(data, size, auto_free);
}

class IndexBufferClass {
 public:
  uint16_t *data;
  int size;

 protected:
  bool auto_free;

 public:
  IndexBufferClass(uint16_t *data, int size, bool auto_free = false)
      : data(data), size(size), auto_free(auto_free) {}

  IndexBufferClass(prim_mode_t mode, int size)
      : data((uint16_t *)xmc_malloc(sizeof(uint16_t) * size, XMC_RAM_CAP_DMA)),
        size(size),
        auto_free(true) {}

  ~IndexBufferClass() {
    if (auto_free) {
      if (data) {
        xmc_free((void *)data);
        data = nullptr;
      }
    }
  }
};

using IndexBuffer = std::shared_ptr<IndexBufferClass>;
static inline IndexBuffer createIndexBuffer(prim_mode_t mode, int size) {
  return std::make_shared<IndexBufferClass>(mode, size);
}

static inline IndexBuffer createIndexBuffer(uint16_t *data, int size,
                                            bool auto_free = false) {
  return std::make_shared<IndexBufferClass>(data, size, auto_free);
}

class MaterialClass {
  colorf base_color = {1.0f, 1.0f, 1.0f, 1.0f};
  colorf emissive_color = {0.0f, 0.0f, 0.0f, 1.0f};
  float metalness = 0.0f;
  float roughness = 0.5f;
};
using Material = std::shared_ptr<MaterialClass>;
static inline Material createMaterial() {
  return std::make_shared<MaterialClass>();
}

class PrimitiveClass {
 public:
  prim_mode_t mode;
  Vec3Buffer position;
  Vec3Buffer normal;
  ColorBuffer color;
  Material material;
  IndexBuffer indexes;
  PrimitiveClass(prim_mode_t mode, Vec3Buffer pos, Vec3Buffer norm,
                 ColorBuffer col, Material mat, IndexBuffer idx = nullptr)
      : mode(mode),
        position(pos),
        normal(norm),
        color(col),
        material(mat),
        indexes(idx) {}

  inline int num_verts() {
    if (indexes) {
      return indexes->size;
    } else if (position) {
      return position->size;
    }
    return 0;
  }

  inline int num_elements() {
    int n = num_verts();
    switch (mode) {
      case prim_mode_t::POINTS: return n;
      case prim_mode_t::LINES: return n / 2;
      case prim_mode_t::LINE_LOOP: return n < 2 ? 0 : n;
      case prim_mode_t::LINE_STRIP: return n < 2 ? 0 : n - 1;
      case prim_mode_t::TRIANGLES: return n / 3;
      case prim_mode_t::TRIANGLE_STRIP: return n < 3 ? 0 : n - 2;
      case prim_mode_t::TRIANGLE_FAN: return n < 3 ? 0 : n - 2;
      default: return 0;
    }
  }

  inline int num_verts_per_element() {
    switch (mode) {
      case prim_mode_t::POINTS: return 1;
      case prim_mode_t::LINES:
      case prim_mode_t::LINE_LOOP:
      case prim_mode_t::LINE_STRIP: return 2;
      case prim_mode_t::TRIANGLES:
      case prim_mode_t::TRIANGLE_STRIP:
      case prim_mode_t::TRIANGLE_FAN: return 3;
      default: return 0;
    }
  }

  inline void fetch_indexes(int *out, int *offset) {
    int n = num_verts();
    switch (mode) {
      case prim_mode_t::POINTS: out[0] = fetch_index(offset, n); break;
      case prim_mode_t::LINES:
        out[0] = fetch_index(offset, n);
        out[1] = fetch_index(offset, n);
        break;
      case prim_mode_t::LINE_LOOP:
        out[0] = fetch_index(offset, n);
        out[1] = fetch_index(offset, n);
        break;
      case prim_mode_t::LINE_STRIP:
        if (*offset == 0) {
          out[0] = fetch_index(offset, n);
          out[1] = fetch_index(offset, n);
        } else {
          out[0] = out[1];
          out[1] = fetch_index(offset, n);
        }
        break;
      case prim_mode_t::TRIANGLES:
        out[0] = fetch_index(offset, n);
        out[1] = fetch_index(offset, n);
        out[2] = fetch_index(offset, n);
        break;
      case prim_mode_t::TRIANGLE_STRIP:
        if (*offset == 0) {
          out[0] = fetch_index(offset, n);
          out[1] = fetch_index(offset, n);
          out[2] = fetch_index(offset, n);
        } else {
          out[0] = out[1];
          out[1] = out[2];
          out[2] = fetch_index(offset, n);
        }
        break;
      case prim_mode_t::TRIANGLE_FAN:
        if (*offset == 0) {
          out[0] = fetch_index(offset, n);
          out[1] = fetch_index(offset, n);
          out[2] = fetch_index(offset, n);
        } else {
          out[1] = out[2];
          out[2] = fetch_index(offset, n);
        }
        break;
    }
  }

 protected:
  inline int fetch_index(int *offset, int n) {
    (*offset)++;
    if (*offset >= n) {
      *offset = 0;
    }
    if (indexes) {
      return indexes->data[*offset];
    } else {
      return *offset;
    }
  }
};
using Primitive = std::shared_ptr<PrimitiveClass>;
static inline Primitive createPrimitive(prim_mode_t mode, Vec3Buffer pos,
                                        Vec3Buffer norm, ColorBuffer col,
                                        Material mat = nullptr,
                                        IndexBuffer idx = nullptr) {
  return std::make_shared<PrimitiveClass>(mode, pos, norm, col, mat, idx);
}

class MeshClass {
 public:
  std::vector<Primitive> primitives;
  MeshClass(std::vector<Primitive> &&prims) : primitives(std::move(prims)) {}
};
using Mesh = std::shared_ptr<MeshClass>;
static inline Mesh createMesh(std::vector<Primitive> &&prims) {
  return std::make_shared<MeshClass>(std::move(prims));
}

static inline Mesh createColoredCube(float s = 1.0f) {
  vec3 v0 = {-s, -s, -s};
  vec3 v1 = {s, -s, -s};
  vec3 v2 = {-s, s, -s};
  vec3 v3 = {s, s, -s};
  vec3 v4 = {-s, -s, s};
  vec3 v5 = {s, -s, s};
  vec3 v6 = {-s, s, s};
  vec3 v7 = {s, s, s};
  vec3 n0 = {-1, 0, 0};
  vec3 n1 = {1, 0, 0};
  vec3 n2 = {0, -1, 0};
  vec3 n3 = {0, 1, 0};
  vec3 n4 = {0, 0, -1};
  vec3 n5 = {0, 0, 1};
  float ca = 1.0f, cb = 0.2f;
  colorf c0 = {ca, cb, cb, 1};
  colorf c1 = {cb, ca, ca, 1};
  colorf c2 = {cb, ca, cb, 1};
  colorf c3 = {ca, cb, ca, 1};
  colorf c4 = {cb, cb, ca, 1};
  colorf c5 = {ca, ca, cb, 1};

  vec3 *poses = new vec3[]{
      v0, v4, v2, v6, v1, v3, v5, v7, v0, v1, v4, v5,
      v2, v6, v3, v7, v0, v2, v1, v3, v4, v5, v6, v7,
  };
  vec3 *norms = new vec3[]{
      n0, n0, n0, n0, n1, n1, n1, n1, n2, n2, n2, n2,
      n3, n3, n3, n3, n4, n4, n4, n4, n5, n5, n5, n5,
  };
  colorf *cols = new colorf[]{
      c0, c0, c0, c0, c1, c1, c1, c1, c2, c2, c2, c2,
      c3, c3, c3, c3, c4, c4, c4, c4, c5, c5, c5, c5,
  };
  uint16_t *idxs = new uint16_t[]{
      0,  1,  2,  2,  1,  3,  4,  5,  6,  6,  5,  7,  8,  9,  10, 10, 9,  11,
      12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23,
  };

  return createMesh({createPrimitive(
      prim_mode_t::TRIANGLES, createVec3Buffer(poses, 24, true),
      createVec3Buffer(norms, 24, true), createColorFBuffer(cols, 24, true),
      nullptr, createIndexBuffer(idxs, 36, true))});
}

}  // namespace xmc

#endif
