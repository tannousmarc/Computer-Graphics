#include "camera.h"
#include "definitions.h"

void reset_camera(Camera &cam){
  cam.focalLength = SCREEN_WIDTH;
  cam.cameraPos = vec4(0,0,-3,1);
  // I4
  cam.cameraRotationY = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
  cam.cameraRotationX = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

  cam.yaw = 0.0;
  cam.pitch = 0.0;
}

void rotateY(Camera& cam, float angle){
  vec4 v1(cos(angle), 0, -sin(angle), 0);
  vec4 v2(0,1,0,0);
  vec4 v3(sin(angle), 0, cos(angle), 0);
  vec4 v4(0, 0, 0, 1);

  mat4 R(v1, v2, v3, v4);
  cam.cameraRotationY = R;
}

void rotateX(Camera& cam, float angle){
  vec4 v1(1, 0, 0, 0);
  vec4 v2(0, cos(angle), sin(angle),0);
  vec4 v3(0, -sin(angle), cos(angle), 0);
  vec4 v4(0, 0, 0, 1);

  mat4 R(v1, v2, v3, v4);
  cam.cameraRotationX = R;
}
