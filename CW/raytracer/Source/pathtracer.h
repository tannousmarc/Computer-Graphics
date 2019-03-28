#ifndef PATHTRACER
#define PATHTRACER

void trace(Ray &ray, const vector<Triangle>& triangles, int depth, vec3& color, Light light);

#endif
