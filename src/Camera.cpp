#include "Camera.h"

glm::mat4 cs200::affineInverse(const glm::mat4 &A) {
  const float determinate = A[0][0] * A[1][1] - A[0][1] * A[1][0];

  glm::mat4 mat = {
      {A[1][1] / determinate, -A[0][1] / determinate, 0.f, 0.f},
      {-A[1][0] / determinate, A[0][0] / determinate, 0.f, 0.f},
      {0.f, 0.f, 1.f, 0.f},
      {0.f, 0.f, 0.f, 1.f} //
  };

  return mat * cs200::translate(-A[3]);
}

cs200::Camera::Camera(const glm::vec4 &C, const glm::vec4 &v, float W, float H) {
  center_point = C;
  up_vector = v;
  right_vector = {-v.y, v.x, 0.f, 1.f};
  rect_width = W;
  rect_height = H;
}

cs200::Camera::Camera() : Camera{{0.f, 0.f, 0.f, 1.f}, {0, 1, 0, 1}, 2.f, 2.f} {}

cs200::Camera &cs200::Camera::moveRight(float x) {
  center_point += right_vector * x;
  return *this;
}

cs200::Camera &cs200::Camera::moveUp(float y) {
  center_point += up_vector * y;
  return *this;
}

cs200::Camera &cs200::Camera::rotate(float t) {
  right_vector = cs200::rotate(t) * right_vector;
  up_vector = {-right_vector.y, right_vector.x, 0.f, 1.f};
  /* up_vector = cs200::rotate(t) * up_vector; */
  return *this;
}

cs200::Camera &cs200::Camera::zoom(float f) {
  rect_width *= f;
  rect_height *= f;
  return *this;
}

glm::mat4 cs200::cameraToWorld(const Camera &cam) {
  return glm::mat4{cam.right(), cam.up(), {0.f, 0.f, 1.f, 0.f}, cam.center()};
}

glm::mat4 cs200::worldToCamera(const Camera &cam) {
  return affineInverse(cameraToWorld(cam));
}

glm::mat4 cs200::cameraToNDC(const Camera &cam) {
  return cs200::scale(1.f / cam.width(), 1.f / cam.height());
}

glm::mat4 cs200::NDCToCamera(const Camera &cam) {
  return cs200::scale(cam.width(), cam.height());
}
