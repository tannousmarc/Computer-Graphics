#ifndef PATHTRACER
#define PATHTRACER

void trace(Ray &ray, const vector<Triangle>& triangles, const vector<Sphere>& spheres, int depth, vec3& color, Light light);

#endif
