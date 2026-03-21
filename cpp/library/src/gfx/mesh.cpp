#include "xmc/gfx/mesh.hpp"

namespace xmc {

Mesh createColoredCube(float s) {
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
  float cl = 1.0f, cd = 0.2f;
  colorf c0 = {cl, cd, cd, 1};
  colorf c1 = {cd, cl, cl, 1};
  colorf c2 = {cd, cl, cd, 1};
  colorf c3 = {cl, cd, cl, 1};
  colorf c4 = {cd, cd, cl, 1};
  colorf c5 = {cl, cl, cd, 1};

  vec3 *poses = new vec3[]{
      v0, v2, v4, v6, v1, v5, v3, v7, v0, v4, v1, v5,
      v2, v3, v6, v7, v0, v1, v2, v3, v4, v6, v5, v7,
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
      PrimitiveMode::TRIANGLES, createVec3Buffer(poses, 24, true),
      createVec3Buffer(norms, 24, true), createColorBuffer(cols, 24, true),
      nullptr, nullptr, createIndexBuffer(idxs, 36, true))});
}

Mesh createSphere(float radius, int segments, int rings, colorf col) {
  Vec3Buffer poses = createVec3Buffer((segments + 1) * (rings + 1));
  Vec3Buffer norms = createVec3Buffer((segments + 1) * (rings + 1));
  ColorBuffer cols = createColorBuffer((segments + 1) * (rings + 1));
  Vec2Buffer uvs = createVec2Buffer((segments + 1) * (rings + 1));
  IndexBuffer idxs = createIndexBuffer(segments * rings * 6);

  for (int i = 0; i <= rings; i++) {
    float v = (float)i / rings;
    float phi = v * M_PI;
    for (int j = 0; j <= segments; j++) {
      float u = (float)j / segments;
      float theta = u * M_PI * 2;
      vec3 pos = {radius * sinf(phi) * cosf(theta), radius * cosf(phi),
                  radius * sinf(phi) * sinf(theta)};
      vec3 norm = pos.normalized();
      poses->data[i * (segments + 1) + j] = pos;
      norms->data[i * (segments + 1) + j] = norm;
      cols->data[i * (segments + 1) + j] = col;
      uvs->data[i * (segments + 1) + j] = {u, v};
    }
  }

  for (int i = 0; i < rings; i++) {
    for (int j = 0; j < segments; j++) {
      uint16_t idx0 = i * (segments + 1) + j;
      uint16_t idx1 = idx0 + segments + 1;
      uint16_t idx2 = idx0 + 1;
      uint16_t idx3 = idx1 + 1;
      idxs->data[i * segments * 6 + j * 6 + 0] = idx0;
      idxs->data[i * segments * 6 + j * 6 + 1] = idx1;
      idxs->data[i * segments * 6 + j * 6 + 2] = idx2;
      idxs->data[i * segments * 6 + j * 6 + 3] = idx2;
      idxs->data[i * segments * 6 + j * 6 + 4] = idx1;
      idxs->data[i * segments * 6 + j * 6 + 5] = idx3;
    }
  }

  return createMesh({createPrimitive(
      PrimitiveMode::TRIANGLES, std::move(poses), std::move(norms),
      std::move(cols), std::move(uvs), nullptr, std::move(idxs))});
}

}  // namespace xmc
