#ifndef SPHERE
#define SPHERE

#include "definitions.h"
#include "ray.h"
#include "material.h"
#include "utils.h"

struct Sphere{
  float radius;
  vec4 center;
  vec3 color;
  Material material;
  Sphere() = default;
  Sphere(float radius, vec4 center, vec3 color, Material material):
    radius(radius), center(center), color(color), material(material) {};


  vec4 normal(const vec4& point) const{
		return normalize(point - center);
	}

  bool intersect(const Ray& ray, Intersection &result) const{
    float t0, t1; // solutions for t if the ray intersects

    // analytic solution
    vec4 L = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2 * dot(ray.direction, L);
    float c = dot(L, L) - radius * radius;
    if (!solveQuadratic(a, b, c, t0, t1)) return false;
    if (t0 > t1) std::swap(t0, t1);

    if (t0 < 0) {
        t0 = t1; // if t0 is negative, let's use t1 instead
        if (t0 < 0) return false; // both t0 and t1 are negative
    }

    result.distance = t0;
    result.position = ray.origin + result.distance * ray.direction;
    result.color = color;
    result.material = material;
    result.normal = normal(result.position);
    return true;
  }
};

#endif
