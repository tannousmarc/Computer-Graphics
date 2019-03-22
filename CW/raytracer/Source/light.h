#ifndef LIGHT
#define LIGHT

vec3 indirectLight = INDIRECT_LIGHT_FACTOR * vec3(1,1,1);

vec3 directLight(Camera& cam, const vector<Triangle>& triangles, Intersection& inter, Light light);

void reset_light(Light &light);

#endif
