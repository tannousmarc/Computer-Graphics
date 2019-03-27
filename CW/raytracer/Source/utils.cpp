#include "utils.h"
#include "definitions.h"

bool checkBoundingBox(float x, float y){
  return x >= 0  && y >= 0 && x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT;
}

// draw uniform samples on a plane and project them to the hemisphere
vec3 diffuseHemisphere(float a, float b){
  const float r = sqrt(1.0f - a * a);
  const float phi = 2 * 3.14 * b;
  return vec3(cos(phi) * r, a, sin(phi) * r);
}

void orthonormalSystem(const vec4& N, vec3& Nt, vec3& Nb){
  if (std::fabs(N.x) > std::fabs(N.y))
    Nt = vec3(N.z, 0, -N.x) / sqrtf(N.x * N.x + N.z * N.z);
  else
    Nt = vec3(0, -N.z, N.y) / sqrtf(N.y * N.y + N.z * N.z);
  Nb = cross(vec3(N.x, N.y, N.z), Nt);
}

void reset_evolutionModel(vec3** pixels, int& samplesSeenSoFar){
  for(int x = 0; x < SCREEN_WIDTH; x++){
    for(int y = 0; y < SCREEN_HEIGHT; y++){
      pixels[x][y] = vec3(0.f, 0.f, 0.f);
    }
  }

  samplesSeenSoFar = 0;
}
