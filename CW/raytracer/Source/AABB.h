#ifndef AABB
#define AABB

#include <algorithm>
#include "definitions.h"

// c++11 needs a lowercase letter here???
struct AABBox
{
    vec3 min;
    vec3 max;
    AABBox() : min(vec3(INFINITE, INFINITE, INFINITE)), max(vec3(-INFINITE, -INFINITE, -INFINITE)){};
    AABBox(vec3 min_, vec3 max_)
    {
        min = min_;
        max = max_;
    }
    bool unbounded() const
    {
        return min.x == -INFINITE || min.y == -INFINITE || min.z == -INFINITE || max.x == INFINITE || max.y == INFINITE || max.z == INFINITE;
    }
    size_t largestDimension() const
    {
        double dx = abs(max.x - min.x);
        double dy = abs(max.y - min.y);
        double dz = abs(max.z - min.z);
        if (dx > dy && dx > dz)
        {
            return 0;
        }
        if (dy > dz)
        {
            return 1;
        }
        return 2;
    }
    // ray-slab tests, see PBRT 2nd edition, section 4.2.1
    bool intersect(const Ray &ray, const vec3 &inverseDirection, double closestKnownT) const
    {
        bool xDirNegative = ray.direction.x < 0;
        bool yDirNegative = ray.direction.y < 0;
        bool zDirNegative = ray.direction.z < 0;

        // check for ray intersection against x and y slabs
        float tmin = ((xDirNegative ? max.x : min.x) - ray.origin.x) * inverseDirection.x;
        float tmax = ((xDirNegative ? min.x : max.x) - ray.origin.x) * inverseDirection.x;
        float tymin = ((yDirNegative ? max.y : min.y) - ray.origin.y) * inverseDirection.y;
        float tymax = ((yDirNegative ? min.y : max.y) - ray.origin.y) * inverseDirection.y;
        if (tmin > tymax || tymin > tmax)
        {
            return false;
        }
        if (tymin > tmin)
        {
            tmin = tymin;
        }
        if (tymax < tmax)
        {
            tmax = tymax;
        }

        // check for ray intersection against z slab
        float tzmin = ((zDirNegative ? max.z : min.z) - ray.origin.z) * inverseDirection.z;
        float tzmax = ((zDirNegative ? min.z : max.z) - ray.origin.z) * inverseDirection.z;
        if (tmin > tzmax || tzmin > tmax)
        {
            return false;
        }
        if (tzmin > tmin)
        {
            tmin = tzmin;
        }
        if (tzmax < tmax)
        {
            tmax = tzmax;
        }
        return (tmin < closestKnownT) && (tmax > EPSILON);
    }
    AABBox enclose(const AABBox &firstBoundingBox, const AABBox &secondBoundingBox)
    {
        AABBox ret;

        ret.min.x = std::min(firstBoundingBox.min.x, secondBoundingBox.min.x);
        ret.min.y = std::min(firstBoundingBox.min.y, secondBoundingBox.min.y);
        ret.min.z = std::min(firstBoundingBox.min.z, secondBoundingBox.min.z);

        ret.max.x = std::max(firstBoundingBox.max.x, secondBoundingBox.max.x);
        ret.max.y = std::max(firstBoundingBox.max.y, secondBoundingBox.max.y);
        ret.max.z = std::max(firstBoundingBox.max.z, secondBoundingBox.max.z);

        return ret;
    }

    AABBox enclose(const AABBox &boundingBox, const vec3 &point)
    {
        AABBox ret;

        ret.min.x = std::min(boundingBox.min.x, point.x);
        ret.min.y = std::min(boundingBox.min.y, point.y);
        ret.min.z = std::min(boundingBox.min.z, point.z);

        ret.max.x = std::max(boundingBox.max.x, point.x);
        ret.max.y = std::max(boundingBox.max.y, point.y);
        ret.max.z = std::max(boundingBox.max.z, point.z);

        return ret;
    };
};

#endif