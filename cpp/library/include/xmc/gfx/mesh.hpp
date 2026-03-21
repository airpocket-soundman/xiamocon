#ifndef XMC_GFX_PRIMITIVE_HPP
#define XMC_GFX_PRIMITIVE_HPP

#include "xmc/geo.hpp"
#include "xmc/gfx/colorf.hpp"
#include "xmc/gfx/gfx_common.hpp"
#include "xmc/gfx/sprite.hpp"
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

using Vec2Buffer = std::shared_ptr<AttributeBufferClass<vec2>>;
static inline Vec2Buffer create_vec2_buffer(int size) {
  return std::make_shared<AttributeBufferClass<vec2>>(size);
}
static inline Vec2Buffer create_vec2_buffer(vec2 *data, int size,
                                            bool auto_free = false) {
  return std::make_shared<AttributeBufferClass<vec2>>(data, size, auto_free);
}

using Vec3Buffer = std::shared_ptr<AttributeBufferClass<vec3>>;
static inline Vec3Buffer create_vec3_buffer(int size) {
  return std::make_shared<AttributeBufferClass<vec3>>(size);
}
static inline Vec3Buffer create_vec3_buffer(vec3 *data, int size,
                                            bool auto_free = false) {
  return std::make_shared<AttributeBufferClass<vec3>>(data, size, auto_free);
}

using ColorBuffer = std::shared_ptr<AttributeBufferClass<colorf>>;
static inline ColorBuffer create_color_buffer(int size) {
  return std::make_shared<AttributeBufferClass<colorf>>(size);
}
static inline ColorBuffer create_color_buffer(colorf *data, int size,
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

  IndexBufferClass(int size)
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
static inline IndexBuffer create_index_buffer(int size) {
  return std::make_shared<IndexBufferClass>(size);
}

static inline IndexBuffer create_index_buffer(uint16_t *data, int size,
                                              bool auto_free = false) {
  return std::make_shared<IndexBufferClass>(data, size, auto_free);
}

class MaterialClass {
 public:
  colorf base_color = {1.0f, 1.0f, 1.0f, 1.0f};
  // colorf emissive_color = {0.0f, 0.0f, 0.0f, 1.0f};
  // float metalness = 0.0f;
  // float roughness = 0.5f;
  Sprite color_texture = nullptr;
};
using Material = std::shared_ptr<MaterialClass>;
static inline Material create_material() {
  return std::make_shared<MaterialClass>();
}

class PrimitiveClass {
 public:
  prim_mode_t mode;
  Vec3Buffer position;
  Vec3Buffer normal;
  ColorBuffer color;
  Vec2Buffer uv;
  Material material;
  IndexBuffer indexes;
  PrimitiveClass(prim_mode_t mode, Vec3Buffer pos, Vec3Buffer norm,
                 ColorBuffer col, Vec2Buffer uv, Material mat,
                 IndexBuffer idx = nullptr)
      : mode(mode),
        position(pos),
        normal(norm),
        color(col),
        uv(uv),
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
};
using Primitive = std::shared_ptr<PrimitiveClass>;
static inline Primitive create_primitive(prim_mode_t mode, Vec3Buffer pos,
                                         Vec3Buffer norm, ColorBuffer col,
                                         Vec2Buffer uv = nullptr,
                                         Material mat = nullptr,
                                         IndexBuffer idx = nullptr) {
  return std::make_shared<PrimitiveClass>(mode, pos, norm, col, uv, mat, idx);
}

class MeshClass {
 public:
  std::vector<Primitive> primitives;
  Material material;
  MeshClass(std::vector<Primitive> &&prims)
      : primitives(std::move(prims)), material(nullptr) {}
  MeshClass(std::vector<Primitive> &&prims, Material &mat)
      : primitives(std::move(prims)), material(mat) {}
};
using Mesh = std::shared_ptr<MeshClass>;
static inline Mesh create_mesh(std::vector<Primitive> &&prims) {
  return std::make_shared<MeshClass>(std::move(prims));
}
static inline Mesh create_mesh(std::vector<Primitive> &&prims, Material mat) {
  return std::make_shared<MeshClass>(std::move(prims), mat);
}

/**
 * @brief Create a colored cube mesh centered at the origin with side length s
 * @param s The side length of the cube
 * @return A Mesh object representing the cube
 */
Mesh create_colored_cube(float s = 1.0f);

/**
 * @brief Create a colored sphere mesh centered at the origin with given radius
 * and tessellation
 * @param radius The radius of the sphere
 * @param segments The number of segments around the equator (longitude)
 * @param rings The number of rings from pole to pole (latitude)
 * @param col The color of the sphere
 * @return A Mesh object representing the sphere
 */
Mesh create_sphere(float radius = 1.0f, int segments = 12, int rings = 6,
                   colorf col = {1.0f, 1.0f, 1.0f, 1.0f});

}  // namespace xmc

#endif
