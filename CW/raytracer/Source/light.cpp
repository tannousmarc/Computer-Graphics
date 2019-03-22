#include "light.h"
#include "intersections.h"

vec3 directLight(Camera& cam, const vector<Triangle>& triangles, Intersection& inter, Light light){
  Intersection light_inter = inter;
  vec4 vecr = normalize(light.lightPos - inter.position);

  float r = glm::length(light.lightPos - inter.position);


  vec3 d = (light.lightColor * max(dot(vecr, normalize(triangles[inter.triangleIndex].normal)), 0.f))
           /
           (float)(4.f * 3.14159 * r * r);

  light_inter.distance = r;

  if(ClosestIntersectionLight(inter.position + vecr * 0.001f, vecr, triangles, light_inter)){

    d = vec3(0.f, 0.f, 0.f);
  }
  return d + indirectLight;
}

void reset_light(Light &light){
  light.lightPos = vec4(0, -0.5, -0.7, 1.0);
  light.lightColor = 14.0f * vec3(1, 1, 1);
}