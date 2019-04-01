#include "utils.h"
#include "definitions.h"
#include "pathtracer.h"

void bounceDiffuse(Ray &ray, const vector<Triangle>& triangles, const vector<Sphere>& spheres, int depth, vec3& color, Light light, Intersection intersection, vec4 surfaceNormal, float atenuare){
  vec3 rotX, rotY;
  orthonormalSystem(surfaceNormal, rotX, rotY);
  vec3 sampledDir = diffuseHemisphere(distribution(generator), distribution(generator));
  vec3 rotatedDir(sampledDir.x * rotY.x + sampledDir.y * surfaceNormal.x + sampledDir.z * rotX.x,
                  sampledDir.x * rotY.y + sampledDir.y * surfaceNormal.y + sampledDir.z * rotX.y,
                  sampledDir.x * rotY.z + sampledDir.y * surfaceNormal.z + sampledDir.z * rotX.z);

  ray.direction = normalize(vec4(rotatedDir, 1));
  ray.origin = ray.origin + ray.direction * 0.001f;

  vec3 temp(0,0,0);
  trace(ray, triangles, spheres, depth + 1, temp, light);
  color = color + (temp * intersection.color) * dot(ray.direction, surfaceNormal) * atenuare * DIFFUSE_MAGIC_NUMBER;
}

void bounceMirror(Ray &ray, const vector<Triangle>& triangles, const vector<Sphere>& spheres, int depth, vec3& color, Light light, vec4 surfaceNormal, float atenuare){
  float costheta = dot(surfaceNormal, ray.direction) * -1.0f;
  ray.direction = normalize(ray.direction + surfaceNormal * (costheta * 2.0f));
  vec3 temp(0.f, 0.f, 0.f);
  trace(ray, triangles, spheres, depth + 1, temp, light);
  color = color + temp * atenuare;
}

void bounceGlass(Ray &ray, const vector<Triangle>& triangles, const vector<Sphere>& spheres, int depth, vec3& color, Light light, vec4 surfaceNormal, float atenuare){
  float R0 = (AIR_INDEX_OF_REFRACTION - GLASS_INDEX_OF_REFRACTION) / (AIR_INDEX_OF_REFRACTION + GLASS_INDEX_OF_REFRACTION);
  R0 = R0 * R0;

  float currRefraction = GLASS_INDEX_OF_REFRACTION;
  // we are inside the glass object
  if(dot(surfaceNormal, ray.direction) > 0){
    surfaceNormal *= -1.0f;
    currRefraction = 1 / currRefraction;
  }
  currRefraction = 1 / currRefraction;

  // wikipedia says cos theta 1 has to be positive
  float cost1 = max(dot(surfaceNormal, ray.direction) * -1.0f, 0.f);
  float cost2 = 1.0f - currRefraction * currRefraction * (1.0f - cost1 * cost1);

  // Schlick approximation for approximating the contribution of the Fresnel factor in the
  // specular reflection of light from a non-conducting interface (surface) between two media.
  float Rprob = R0 + (1.0f - R0) * pow(1.0f - cost1, 5.0f);

  // refraction direction
  if (cost2 > 0 && distribution(generator) > Rprob) {
      // Snell's law vector from
      ray.direction = normalize((ray.direction * currRefraction) + (surfaceNormal * (currRefraction * cost1 - sqrt(cost2))));
  }
  // reflection direction
  else {
      // Snell's law vector from
      ray.direction = normalize(ray.direction + surfaceNormal * (cost1 * 2.0f));
  }

  vec3 tmp(0.f, 0.f, 0.f);
  trace(ray, triangles, spheres, depth + 1, tmp, light);
  color = color + tmp * atenuare * GLASS_MAGIC_NUMBER;
}

void bounceSpecular(Ray &ray, const vector<Triangle>& triangles, const vector<Sphere>& spheres, int depth, vec3& color, Light light, vec4 surfaceNormal, float atenuare){
  ray.direction =  normalize(ray.direction - surfaceNormal * (dot(ray.direction, surfaceNormal) * 2.f));
  vec3 temp(0,0,0);
  trace(ray, triangles, spheres, depth + 1, temp, light);
  color = color + temp;
}

void trace(Ray &ray, const vector<Triangle>& triangles, const vector<Sphere>& spheres, int depth, vec3& color, Light light){
  Intersection intersection;
  if(closestIntersection(ray, triangles, spheres, intersection) == false)
    return;

  float atenuare = 1.0;
  // base case
  const float russianRoulette = RUSSIAN_ROULETTE_FACTOR;
  if (depth >= MAX_DEPTH || distribution(generator) <= russianRoulette) {
    Light softShadowLight;
    softShadowLight.lightPos.x = light.lightPos.x + (distribution(generator) * 2 - 1) * 0.1f;
    softShadowLight.lightPos.y = light.lightPos.y;
    softShadowLight.lightPos.z = light.lightPos.z + (distribution(generator) * 2 - 1) * 0.1f;
    softShadowLight.lightPos.w = light.lightPos.w;
    softShadowLight.lightColor = light.lightColor;
    // glass/mirror doesn't have a color
    if(strcmp(intersection.material.type, "glass") == 0){
      color = directLight(triangles, intersection, softShadowLight);
    }
    else{
      color = intersection.color * directLight(triangles, intersection, softShadowLight) * atenuare;
    }
    atenuare = 1.0f / (1.0f - russianRoulette);
    return;

  }

  vec4 surfaceNormal = intersection.normal;
  ray.origin = intersection.position;

  // recursive cases
  if(strcmp(intersection.material.type, "diffuse") == 0)
    bounceDiffuse(ray, triangles, spheres, depth, color, light, intersection, surfaceNormal, atenuare);
  else if(strcmp(intersection.material.type, "mirror") == 0)
    bounceMirror(ray, triangles, spheres, depth, color, light, surfaceNormal, atenuare);
  else if(strcmp(intersection.material.type, "glass") == 0)
    bounceGlass(ray, triangles, spheres, depth, color, light, surfaceNormal, atenuare);
  else if(strcmp(intersection.material.type, "specular") == 0)
    bounceSpecular(ray, triangles, spheres, depth, color, light, surfaceNormal, atenuare);
}
