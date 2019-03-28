#ifndef UTILS
#define UTILS

bool checkBoundingBox(float x, float y);

void orthonormalSystem(const vec4& N, vec3& Nt, vec3& Nb);

void reset_evolutionModel(vec3** pixels, int& samplesSeenSoFar);

void trace(Ray &ray, const vector<Triangle>& triangles, int depth, vec3& color, Light light);

#endif
