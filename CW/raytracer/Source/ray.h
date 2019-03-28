#ifndef RAY
#define RAY

#include "definitions.h"

struct Ray{
  vec4 origin;
  vec4 direction;

  Ray() = default;
  Ray(vec4 origin, vec4 direction):
    origin(origin), direction(normalize(direction)) {};
};

#endif
