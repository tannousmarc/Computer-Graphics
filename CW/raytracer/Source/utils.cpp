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
