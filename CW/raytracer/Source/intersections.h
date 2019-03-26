#ifndef INTERSECTIONS
#define INTERSECTIONS

#include "ray.h"

bool closestIntersection(const Ray &ray,
    const vector<Triangle>& triangles,
    Intersection& closestIntersection);

bool existsIntersection(const Ray &ray,
    const vector<Triangle>& triangles, Intersection inter);

#endif
