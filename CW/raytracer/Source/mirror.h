#ifndef MIRROR
#define MIRROR

#include "definitions.h"

vec3 mirror(Camera& cam, const vector<Triangle>& triangles, Intersection& inter, Light light, vec4 direction, int bounces);

#endif
