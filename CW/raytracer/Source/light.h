#ifndef LIGHT
#define LIGHT

vec3 directLight(const vector<Triangle>& triangles, Intersection& inter, Light light);

void reset_light(Light &light);

#endif
