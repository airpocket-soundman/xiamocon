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
  bool autoFree;

 public:
  AttributeBufferClass(T *data, int size, bool autoFree = false)
      : data(data), size(size), autoFree(autoFree) {}

  AttributeBufferClass(int size)
      : data((T *)xmcMalloc(sizeof(T) * size, XMC_RAM_CAP_DMA)),
        size(size),
        autoFree(true) {}

  ~AttributeBufferClass() {
    if (autoFree) {
      if (data) {
        xmcFree((void *)data);
        data = nullptr;
      }
    }
  }
};

using Vec2Buffer = std::shared_ptr<AttributeBufferClass<vec2>>;
static inline Vec2Buffer createVec2Buffer(int size) {
  return std::make_shared<AttributeBufferClass<vec2>>(size);
}
static inline Vec2Buffer createVec2Buffer(vec2 *data, int size,
                                            bool autoFree = false) {
  return std::make_shared<AttributeBufferClass<vec2>>(data, size, autoFree);
}

using Vec3Buffer = std::shared_ptr<AttributeBufferClass<vec3>>;
static inline Vec3Buffer createVec3Buffer(int size) {
  return std::make_shared<AttributeBufferClass<vec3>>(size);
}
static inline Vec3Buffer createVec3Buffer(vec3 *data, int size,
                                            bool autoFree = false) {
  return std::make_shared<AttributeBufferClass<vec3>>(data, size, autoFree);
}

using ColorBuffer = std::shared_ptr<AttributeBufferClass<colorf>>;
static inline ColorBuffer createColorBuffer(int size) {
  return std::make_shared<AttributeBufferClass<colorf>>(size);
}
static inline ColorBuffer createColorBuffer(colorf *data, int size,
                                              bool autoFree = false) {
  return std::make_shared<AttributeBufferClass<colorf>>(data, size, autoFree);
}

class IndexBufferClass {
 public:
  uint16_t *data;
  int size;

 protected:
  bool autoFree;

 public:
  IndexBufferClass(uint16_t *data, int size, bool autoFree = false)
      : data(data), size(size), autoFree(autoFree) {}

  IndexBufferClass(int size)
      : data((uint16_t *)xmcMalloc(sizeof(uint16_t) * size, XMC_RAM_CAP_DMA)),
        size(size),
        autoFree(true) {}

  ~IndexBufferClass() {
    if (autoFree) {
      if (data) {
        xmcFree((void *)data);
        data = nullptr;
      }
    }
  }
};

using IndexBuffer = std::shared_ptr<IndexBufferClass>;
static inline IndexBuffer createIndexBuffer(int size) {
  return std::make_shared<IndexBufferClass>(size);
}

static inline IndexBuffer createIndexBuffer(uint16_t *data, int size,
                                              bool autoFree = false) {
  return std::make_shared<IndexBufferClass>(data, size, autoFree);
}

class MaterialClass {
 public:
  colorf baseColor = {1.0f, 1.0f, 1.0f, 1.0f};
  // colorf emissiveColor = {0.0f, 0.0f, 0.0f, 1.0f};
  // float metalness = 0.0f;
  // float roughness = 0.5f;
  Sprite colorTexture = nullptr;
};
using Material = std::shared_ptr<MaterialClass>;
static inline Material createMaterial() {
  return std::make_shared<MaterialClass>();
}

class PrimitiveClass {
 public:
  PrimitiveMode mode;
  Vec3Buffer position;
  Vec3Buffer normal;
  ColorBuffer color;
  Vec2Buffer uv;
  Material material;
  IndexBuffer indexes;
  PrimitiveClass(PrimitiveMode mode, Vec3Buffer pos, Vec3Buffer norm,
                 ColorBuffer col, Vec2Buffer uv, Material mat,
                 IndexBuffer idx = nullptr)
      : mode(mode),
        position(pos),
        normal(norm),
        color(col),
        uv(uv),
        material(mat),
        indexes(idx) {}

  inline int numVertices() {
    if (indexes) {
      return indexes->size;
    } else if (position) {
      return position->size;
    }
    return 0;
  }

  inline int numElements() {
    int n = numVertices();
    switch (mode) {
      case PrimitiveMode::POINTS: return n;
      case PrimitiveMode::LINES: return n / 2;
      case PrimitiveMode::LINE_LOOP: return n < 2 ? 0 : n;
      case PrimitiveMode::LINE_STRIP: return n < 2 ? 0 : n - 1;
      case PrimitiveMode::TRIANGLES: return n / 3;
      case PrimitiveMode::TRIANGLE_STRIP: return n < 3 ? 0 : n - 2;
      case PrimitiveMode::TRIANGLE_FAN: return n < 3 ? 0 : n - 2;
      default: return 0;
    }
  }
};
using Primitive = std::shared_ptr<PrimitiveClass>;
static inline Primitive createPrimitive(PrimitiveMode mode, Vec3Buffer pos,
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
static inline Mesh createMesh(std::vector<Primitive> &&prims) {
  return std::make_shared<MeshClass>(std::move(prims));
}
static inline Mesh createMesh(std::vector<Primitive> &&prims, Material mat) {
  return std::make_shared<MeshClass>(std::move(prims), mat);
}

/**
 * @brief Create a colored cube mesh centered at the origin with side length s
 * @param s The side length of the cube
 * @return A Mesh object representing the cube
 */
Mesh createColoredCube(float s = 1.0f);

/**
 * @brief Create a colored sphere mesh centered at the origin with given radius
 * and tessellation
 * @param radius The radius of the sphere
 * @param segments The number of segments around the equator (longitude)
 * @param rings The number of rings from pole to pole (latitude)
 * @param col The color of the sphere
 * @return A Mesh object representing the sphere
 */
Mesh createSphere(float radius = 1.0f, int segments = 12, int rings = 6,
                   colorf col = {1.0f, 1.0f, 1.0f, 1.0f});

}  // namespace xmc

#endif
