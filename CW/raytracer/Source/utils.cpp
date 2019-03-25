#include "utils.h"
#include "definitions.h"

bool checkBoundingBox(float x, float y){
  return x >= 0  && y >= 0 && x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT;
}

// draw uniform samples on a plane and project them to the hemisphere
vec3 diffuseHemisphere(float a, float b){
  const float r = sqrt(1.0f - a * a);
  const float phi = 2 * 3.14 * b;
  return vec3(cos(phi) * r, sin(phi) * r, a);
}

void orthonormalSystem(const vec4& v1, vec3& v2, vec3& v3){
  if(abs(v1.x) > abs(v1.y)){
    float invLen = 1.f / sqrtf(v1.x * v1.x + v1.z * v1.z);
    v2 = vec3(-v1.z * invLen, 0.0f, v1.x * invLen);
  }
  else{
    float invLen = 1.0f / sqrtf(v1.y * v1.y + v1.z * v1.z);
    v2 = vec3(0.0f, v1.z * invLen, -v1.y * invLen);
  }
  v3.x = v1.y * v2.z - v1.z * v2.y;
  v3.y = v1.z * v2.x - v1.x * v2.z;
  v3.z = v1.x * v2.y - v1.y * v2.x;
}
