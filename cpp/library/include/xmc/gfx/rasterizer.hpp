#ifndef XMC_GFX_RASTERIZER_HPP
#define XMC_GFX_RASTERIZER_HPP

#include "xmc/geo.hpp"
#include "xmc/gfx/colorf.hpp"
#include "xmc/gfx/mesh.hpp"
#include "xmc/gfx/sprite.hpp"

namespace xmc {

using depth_t = uint8_t;

struct baked_vertex_t {
  vec3 pos;
  colorf color;
  vec2 uv;
};

class RasterizerClass;
using Rasterizer = std::shared_ptr<RasterizerClass>;

static inline Rasterizer create_rasterizer(int width, int height,
                                           uint32_t stack_size = 16) {
  return std::make_shared<RasterizerClass>(width, height, stack_size);
}

class RasterizerClass {
 private:
  const int width;
  const int height;
  const int stack_size;

  depth_t *depth_buff;

  Sprite target;
  rect_t viewport;

  mat4 projection_matrix;

  mat4 *matrix_stack;
  int matrix_stack_top = 0;
  float z_near = 0.01f;
  float z_far = 100.0f;

  colorf env_light = {0.1f, 0.1f, 0.1f, 1.0f};

  vec3 parallel_light_dir = vec3(0.5f, 0.5f, 1.0f).normalized();
  colorf parallel_light_color = {0.8f, 0.8f, 0.8f, 1.0f};

  Material material;

 public:
  RasterizerClass(int w, int h, uint32_t stack_size)
      : width(w), height(h), stack_size(stack_size) {
    depth_buff = (depth_t *)xmc_malloc(sizeof(depth_t) * width * height,
                                       XMC_RAM_CAP_DMA);
    matrix_stack =
        (mat4 *)xmc_malloc(sizeof(mat4) * stack_size, XMC_RAM_CAP_DMA);
  };

  ~RasterizerClass() {
    if (depth_buff) {
      xmc_free(depth_buff);
      depth_buff = nullptr;
    }
    if (matrix_stack) {
      xmc_free(matrix_stack);
      matrix_stack = nullptr;
    }
  }

  inline void set_target(Sprite &target, rect_t viewport) {
    this->target = target;
    this->viewport = viewport;
  }

  inline void set_target(Sprite &target) {
    set_target(target, rect_t{0, 0, target->width(), target->height()});
  }

  inline void set_parallel_light(const vec3 &dir, const colorf &color) {
    parallel_light_dir = dir.normalized();
    parallel_light_color = color;
  }

  inline void set_projection(const mat4 &proj) { projection_matrix = proj; }
  inline const mat4 &get_projection() const { return projection_matrix; }

  inline void load_matrix(const mat4 &proj) {
    matrix_stack[matrix_stack_top] = proj;
  }

  inline void load_identity() {
    matrix_stack[matrix_stack_top] = mat4::identity();
  }

  inline void push_matrix() {
    if (matrix_stack_top < stack_size - 1) {
      matrix_stack_top++;
      matrix_stack[matrix_stack_top] = matrix_stack[matrix_stack_top - 1];
    }
  }

  inline void pop_matrix() {
    if (matrix_stack_top > 0) {
      matrix_stack_top--;
    }
  }

  inline mat4 &current_matrix() { return matrix_stack[matrix_stack_top]; }
  inline void translate(const vec3 &t) { current_matrix().translate(t); }
  inline void translate(float x, float y, float z) {
    current_matrix().translate(x, y, z);
  }
  inline void rotate(const quat &q) { current_matrix().rotate(q); }
  inline void rotate(float pitch, float roll, float yaw) {
    current_matrix().rotate(pitch, roll, yaw);
  }
  inline void rotate(const vec3 &axis, float angle) {
    current_matrix().rotate(axis, angle);
  }
  inline void scale(const vec3 &s) { current_matrix().scale(s); }
  inline void scale(float s) { current_matrix().scale(s); }

  inline void set_depth_range(float near, float far) {
    z_near = near;
    z_far = far;
  }

  inline void set_material(const Material &mat) { material = mat; }

  void clear_depth(depth_t value = 0xFF);

  void render_mesh(const Mesh &mesh);

  inline void render_primitive(const Primitive &prim) {
    render_primitive(prim, material);
  }

  void render_primitive(const Primitive &prim, const Material &mat);

  void render_triangle(const baked_vertex_t &v0, const baked_vertex_t &v1,
                       const baked_vertex_t &v2, const Material &mat);
};

}  // namespace xmc

#endif
