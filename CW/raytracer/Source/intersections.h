#ifndef INTERSECTIONS
#define INTERSECTIONS

#include "ray.h"
#include "sphere.h"

bool closestIntersection(const Ray &ray,
    const vector<Triangle>& triangles,
    const vector<Sphere>& spheres,
    Intersection& closestIntersection);

bool existsIntersection(const Ray &ray,
    const vector<Triangle>& triangles, Intersection inter);

#endif
