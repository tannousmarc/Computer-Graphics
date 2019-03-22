#include "mirror.h"
#include "light.h"

vec3 mirror(Camera& cam, const vector<Triangle>& triangles, Intersection& inter, Light light, vec4 direction, int bounces){
  Intersection mirror_inter;
  direction = normalize(direction);
  vec4 dir;
  vec3 color(0,0,0);
  bounces++;
  float c = -dot(direction, triangles[inter.triangleIndex].normal);
  dir = direction + (2.0f * triangles[inter.triangleIndex].normal * c);
  dir = normalize(dir);
  if(bounces > BOUNCES_THRESHOLD){
    vec3 lightIntensity = directLight(cam, triangles, inter, light);
    return triangles[inter.triangleIndex].color * lightIntensity;
  }
  Camera newCam;
  newCam.cameraPos = inter.position + dir * 0.001f;

  if(ClosestIntersection(newCam, dir, triangles, mirror_inter)){
    if(triangles[mirror_inter.triangleIndex].isMirror){
      color = mirror(cam, triangles, mirror_inter, light, dir, bounces);
    }
    else{
      color = triangles[mirror_inter.triangleIndex].color;
    }
    vec3 lightIntensity;

    lightIntensity = directLight(cam, triangles, mirror_inter, light);
    return lightIntensity * color;
  }
  return color;
}
