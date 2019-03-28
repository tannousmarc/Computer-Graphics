#include "intersections.h"
#include "definitions.h"
#include "ray.h"
#include "sphere.h"

bool closestIntersection(const Ray &ray, const vector<Triangle>& triangles, const vector<Sphere>& spheres,
                         Intersection& closestIntersection){
  float minimumDistance = std::numeric_limits<float>::max();
  bool okay = false;

  for(unsigned int i = 0; i < triangles.size(); i++){
    Triangle triangle = triangles[i];
    vec4 v0 = triangle.v0;
    vec4 v1 = triangle.v1;
    vec4 v2 = triangle.v2;

    vec3 e1 = vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    vec3 e2 = vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
    vec3 b = vec3(ray.origin.x - v0.x, ray.origin.y - v0.y, ray.origin.z - v0.z);
    mat3 A(vec3(-ray.direction), e1, e2);
    mat3 m = A;
    m[0] = b;
    float t = glm::determinant(m) / glm::determinant(A);

    if(t >= 0 && t <= minimumDistance){
      vec3 x = glm::inverse( A ) * b;
      if(x.y >= 0 && x.z >= 0 && (x.y + x.z <= 1.001)){
        minimumDistance = abs(x.x);
        closestIntersection.position = v0 + x.y*vec4(e1.x, e1.y, e1.z, 0) + x.z*vec4(e2.x, e2.y, e2.z, 0);
        closestIntersection.distance = abs(x.x);
        closestIntersection.color = triangles[i].color;
        closestIntersection.normal = triangles[i].normal;
        closestIntersection.material = triangles[i].material;
        okay = true;
      }
    }
  }

  for(unsigned int i = 0; i < spheres.size(); i++){
    Intersection result;
    bool valid = spheres[i].intersect(ray, result);
    if(valid && result.distance >= EPSILON && result.distance <= minimumDistance){

      minimumDistance = result.distance;
      closestIntersection.position = result.position;
      closestIntersection.distance = result.distance;
      closestIntersection.color = result.color;
      closestIntersection.normal = result.normal;
      closestIntersection.material = result.material;
      okay = true;
    }
  }

  return okay;
};

bool existsIntersection(const Ray &ray, const vector<Triangle>& triangles,
                              Intersection inter){
  bool okay = false;

  for(unsigned int i = 0; i < triangles.size(); i++){
    Triangle triangle = triangles[i];
    vec4 v0 = triangle.v0;
    vec4 v1 = triangle.v1;
    vec4 v2 = triangle.v2;

    vec3 e1 = vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    vec3 e2 = vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
    vec3 b = vec3(ray.origin.x - v0.x, ray.origin.y - v0.y, ray.origin.z - v0.z);
    mat3 A(vec3(-ray.direction), e1, e2);

    mat3 m = A;
    m[0] = b;
    float t = glm::determinant(m) / glm::determinant(A);
    if(t >= 0 && t <= inter.distance){
      vec3 x = glm::inverse( A ) * b;
      if(x.y >= 0 && x.z >= 0 && (x.y + x.z <= 1)){
        okay = true;
        break;
      }
    }
  }

  return okay;
};
