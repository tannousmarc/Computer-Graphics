#ifndef INTERSECTIONS
#define INTERSECTIONS

bool ClosestIntersection(Camera& cam, vec4 dir,
    const vector<Triangle>& triangles,
    Intersection& closestIntersection);

bool ClosestIntersectionLight( vec4 start, vec4 dir,
    const vector<Triangle>& triangles, Intersection inter);

#endif
