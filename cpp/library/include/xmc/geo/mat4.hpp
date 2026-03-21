/**
 * @file mat4.hpp
 * @brief 4x4 matrix for 3D transformations
 */

#ifndef XMC_GEO_MAT4_HPP
#define XMC_GEO_MAT4_HPP

#include "xmc/geo/geo_common.hpp"
#include "xmc/geo/quat.hpp"
#include "xmc/geo/vec3.hpp"

#include <math.h>

namespace xmc {

struct mat4 {
  float m[16];

  mat4() {
    for (int i = 0; i < 16; i++) {
      m[i] = 0;
    }
  }

  /**
   * @return an identity matrix
   */
  static mat4 identity() {
    mat4 result;
    result.m[0] = 1;
    result.m[5] = 1;
    result.m[10] = 1;
    result.m[15] = 1;
    return result;
  }

  /**
   * @brief Load the identity matrix into this matrix
   * After calling this function, this matrix will be an identity matrix
   */
  inline void loadIdentity() { *this = identity(); }

  /**
   * @brief Create a rotation matrix from a quaternion
   * @param q The quaternion representing the rotation
   * @return A 4x4 rotation matrix
   */
  static mat4 fromQuat(const quat &q) {
    mat4 result;
    float xx = q.x * q.x;
    float yy = q.y * q.y;
    float zz = q.z * q.z;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float yz = q.y * q.z;
    float wx = q.w * q.x;
    float wy = q.w * q.y;
    float wz = q.w * q.z;

    result.m[0] = 1 - 2 * (yy + zz);
    result.m[1] = 2 * (xy + wz);
    result.m[2] = 2 * (xz - wy);
    result.m[3] = 0;

    result.m[4] = 2 * (xy - wz);
    result.m[5] = 1 - 2 * (xx + zz);
    result.m[6] = 2 * (yz + wx);
    result.m[7] = 0;

    result.m[8] = 2 * (xz + wy);
    result.m[9] = 2 * (yz - wx);
    result.m[10] = 1 - 2 * (xx + yy);
    result.m[11] = 0;

    result.m[12] = 0;
    result.m[13] = 0;
    result.m[14] = 0;
    result.m[15] = 1;

    return result;
  }

  mat4 operator*(const mat4 &other) const {
    mat4 result;
    for (int col = 0; col < 4; col++) {
      for (int row = 0; row < 4; row++) {
        float sum = 0;
        for (int k = 0; k < 4; k++) {
          sum += m[k * 4 + row] * other.m[col * 4 + k];
        }
        result.m[col * 4 + row] = sum;
      }
    }
    return result;
  }

  /**
   * @brief Transform a point by this matrix, applying perspective division
   * @param v The point to transform
   * @return The transformed point
   * @note If the w component after transformation is close to zero, returns
   * (0, 0, 0) to avoid division by zero
   */
  inline vec3 transform(const vec3 &v) const {
    float rx = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12];
    float ry = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13];
    float rz = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];
    float rw = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15];
    if (fabsf(rw) < 1e-8f) {
      return vec3(0, 0, 0);
    }
    float inv_w = 1.0f / rw;
    return vec3(rx * inv_w, ry * inv_w, rz * inv_w);
  }

  inline void translate(const vec3 &t) {
    m[12] += t.x;
    m[13] += t.y;
    m[14] += t.z;
  }

  inline void translate(float x, float y, float z) {
    m[12] += x;
    m[13] += y;
    m[14] += z;
  }

  inline void rotate(const quat &q) { *this = fromQuat(q) * (*this); }

  inline void rotate(float pitch, float roll, float yaw) {
    rotate(quat::fromEuler(pitch, roll, yaw));
  }

  inline void rotate(const vec3 &axis, float angle) {
    rotate(quat::fromAxisAngle(axis, angle));
  }

  inline void scale(const vec3 &s) {
    m[0] *= s.x;
    m[5] *= s.y;
    m[10] *= s.z;
  }

  inline void scale(float s) {
    m[0] *= s;
    m[5] *= s;
    m[10] *= s;
  }
};

}  // namespace xmc

#endif
