#include "mirror.h"
#include "light.h"

vec3 mirror(const vector<Triangle>& triangles, Intersection& inter, Light light, vec4 direction, int bounces){
  bounces++;

  Intersection mirrorInter;
  vec4 dir;

  // default color is black: if the light bounces and can't find a surface,
  //                         then it must be that the ray went out of the cornell
  //                         box, thus the color is black.
  vec3 color(0,0,0);

  float c = -dot(direction, triangles[inter.triangleIndex].normal);

  dir = normalize(direction + (2.0f * triangles[inter.triangleIndex].normal * c));

  if(bounces > BOUNCES_THRESHOLD){
    vec3 lightIntensity = directLight(triangles, inter, light);
    return triangles[inter.triangleIndex].color * lightIntensity;
  }

  Ray ray(inter.position + dir * 0.001f, dir);

  if(closestIntersection(ray, triangles, mirrorInter)){
    if(triangles[mirrorInter.triangleIndex].isMirror){
      color = mirror(triangles, mirrorInter, light, dir, bounces);
    }
    else{
      color = triangles[mirrorInter.triangleIndex].color;
    }
    vec3 lightIntensity;

    lightIntensity = directLight(triangles, mirrorInter, light);
    return lightIntensity * color;
  }
  return color;
}
