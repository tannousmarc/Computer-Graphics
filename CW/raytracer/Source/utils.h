#ifndef UTILS
#define UTILS
#include "sphere.h"
bool checkBoundingBox(float x, float y);

void orthonormalSystem(const vec4& N, vec3& Nt, vec3& Nb);

void reset_evolutionModel(vec3** pixels, int& samplesSeenSoFar);

bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1);

#endif
