#include "mirror.h"
#include "light.h"

vec3 mirror(Camera& cam, const vector<Triangle>& triangles, Intersection& inter, Light light, vec4 direction, int bounces){
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
    vec3 lightIntensity = directLight(cam, triangles, inter, light);
    return triangles[inter.triangleIndex].color * lightIntensity;
  }

  Camera newCam;
  newCam.cameraPos = inter.position + dir * 0.01f;

  if(ClosestIntersection(newCam, dir, triangles, mirrorInter)){
    if(triangles[mirrorInter.triangleIndex].isMirror){
      color = mirror(cam, triangles, mirrorInter, light, dir, bounces);
    }
    else{
      color = triangles[mirrorInter.triangleIndex].color;
    }
    vec3 lightIntensity;

    lightIntensity = directLight(cam, triangles, mirrorInter, light);
    return lightIntensity * color;
  }
  return color;
}
