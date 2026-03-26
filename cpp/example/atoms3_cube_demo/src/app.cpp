#include "xmc/app.hpp"
#include "xmc/display.hpp"
#include "xmc/geo.hpp"
#include "xmc/gfx.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/input.hpp"

#include <math.h>

#include "mpu6886.hpp"

namespace xmc {

static mpu6886::SensorI2C imu;
static quat imu_pos;

static Sprite screen = create_sprite565(display::WIDTH, display::HEIGHT);
static Mesh cube     = createColoredCube();
static Rasterizer rasterizer =
    createRasterizer(display::WIDTH, display::HEIGHT);

static uint64_t last_imu_update_us = 0;

static void update_imu_position(quat *p, const float *values, float dt);
static void create_projection_matrix(mat4 *M, const quat *Q, const vec3 *E,
                                     float rw, float rh, int sw, int sh);

AppConfig appGetConfig() {
  auto cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = display::InterfaceFormat::RGB565;
  return cfg;
}

void appSetup() {
  screen->clear(0);
  imu_pos = {1, 0, 0, 0};
  imu.init();
}

void appLoop() {
  float values[7] = {0};
  uint64_t now_us = getTimeUs();
  float dt = 0.0f;
  if (last_imu_update_us != 0) {
    dt = (float)(now_us - last_imu_update_us) * 1e-6f;
  }
  last_imu_update_us = now_us;

  XmcStatus sts = imu.read_sensor(values);
  if (sts == XMC_OK && dt > 0.0f) {
    update_imu_position(&imu_pos, values, dt);
  }

  // Btn A: reset orientation
  if (input::wasPressed(input::Button::A)) {
    imu_pos = {1, 0, 0, 0};
  }

  screen->completeTransfer();
  screen->clear(0);

  // Build projection matrix from IMU orientation
  mat4 proj;
  vec3 eye_pos = {0, 0.1f, 0.15f};
  create_projection_matrix(&proj, &imu_pos, &eye_pos,
                           0.03f, 0.03f,
                           display::WIDTH, display::HEIGHT);

  // Render cube
  rasterizer->setTarget(screen);
  rasterizer->clearDepth();
  rasterizer->setDepthRange(-1.0f, 1.0f);

  vec3 light_dir = {0, 0.5f, 1.0f};
  light_dir = imu_pos.conjugate().rotate(light_dir);
  rasterizer->setParallelLight(light_dir, colorf(2.0f, 2.0f, 2.0f, 1.0f));

  rasterizer->setProjection(proj);
  rasterizer->loadIdentity();
  rasterizer->pushMatrix();
  rasterizer->scale(0.012f);
  rasterizer->renderMesh(cube);
  rasterizer->popMatrix();

  screen->startTransferToDisplay(0, 0);
}

// ---- IMU complementary filter -----------------------------------------------

static void update_imu_position(quat *p, const float *values, float dt) {
  constexpr float DEG_TO_RAD = (float)(M_PI / 180.0);
  float gx = values[1] * DEG_TO_RAD;
  float gy = values[2] * DEG_TO_RAD;
  float gz = values[3] * DEG_TO_RAD;
  float ax = values[4];
  float ay = values[5];
  float az = values[6];

  // Gyroscope integration (small-angle approximation)
  float half_dt = dt * 0.5f;
  quat q_gyro(1.0f, gx * half_dt, gy * half_dt, gz * half_dt);
  *p = (*p * q_gyro).normalized();

  // Accelerometer tilt correction (valid only near 1G)
  float a_len = sqrtf(ax * ax + ay * ay + az * az);
  if (a_len > 0.9f && a_len < 1.1f) {
    vec3 a_meas(ax / a_len, ay / a_len, az / a_len);
    vec3 g_world(0, 0, 1);
    vec3 g_body = p->conjugate().rotate(g_world);

    vec3 cross = a_meas.cross(g_body);
    float cross_len = cross.length();
    float dot = a_meas.dot(g_body);

    if (cross_len > 1e-6f) {
      float err_angle = atan2f(cross_len, dot);
      constexpr float ALPHA = 0.02f;
      quat q_corr =
          quat::fromAxisAngle(cross * (1.0f / cross_len), err_angle * ALPHA);
      *p = (q_corr * (*p)).normalized();
    }
  }
}

// ---- Perspective projection matrix ------------------------------------------

static void create_projection_matrix(mat4 *M, const quat *Q, const vec3 *E,
                                     float rw, float rh, int sw, int sh) {
  vec3 e = Q->conjugate().rotate(*E);

  float px_per_m_x = (float)sw / rw;
  float px_per_m_y = (float)sh / rh;
  float hw = (float)sw * 0.5f;
  float hh = (float)sh * 0.5f;

  mat4 result;
  result.m[0]  = e.z * px_per_m_x;
  result.m[5]  = -e.z * px_per_m_y;
  result.m[8]  = -e.x * px_per_m_x - hw;
  result.m[9]  = -e.y * px_per_m_y - hh;
  result.m[10] = -1.0f;
  result.m[11] = -1.0f;
  result.m[12] = e.z * hw;
  result.m[13] = e.z * hh;
  result.m[15] = e.z;
  *M = result;
}

}  // namespace xmc
