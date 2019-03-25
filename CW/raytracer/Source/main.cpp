#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <random>

#include "definitions.h"
#include "light.cpp"
#include "intersections.cpp"
#include "mirror.cpp"
#include "camera.cpp"
#include "keyboard.cpp"
#include "utils.cpp"
#include "ray.h"

// Helpers for random generation
std::mt19937 mersenneTwister;
std::uniform_real_distribution<float> uniform;

#define RND (2.0 * uniform(mersenneTwister) - 1.0)
#define RND2 (uniform(mersenneTwister))
//Extensions: Kramer's rule, Mirrors

int main(int argc, char* argv[])
{
  srand (time(NULL));
  vector<Triangle> triangles;
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  LoadTestModel(triangles);
  Camera cam;
  reset_camera(cam);
  Light light;
  reset_light(light);

  while( NoQuitMessageSDL() )
    {
      Update(cam, light);
      Draw(screen, triangles, cam, light);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

void trace(Ray &ray, const vector<Triangle>& triangles, int depth, vec3& color){
  //Russian roulette factor
  float rrFactor = 1.0;
  if(depth >= 5){
    const float rrStopProbability = 0.1;
    if(RND2 <= rrStopProbability){
      return;
    }
    rrFactor = 1.0 / (1.0 - rrStopProbability);
  }

  Intersection intersection;
  if(closestIntersection(ray, triangles, intersection) == false)
    return;
  vec3 hitPoint = vec3(ray.origin.x, ray.origin.y, ray.origin.z) +
                  vec3(ray.direction.x, ray.direction.y, ray.direction.z) * intersection.distance;
  vec4 surfaceNormal = triangles[intersection.triangleIndex].normal;
  ray.origin.x = hitPoint.x;
  ray.origin.y = hitPoint.y;
  ray.origin.z = hitPoint.z;

  const float emission = triangles[intersection.triangleIndex].material.emission;
  color = color + vec3(emission, emission, emission) * rrFactor;
  if(strcmp(triangles[intersection.triangleIndex].material.type, "diffuse")){
      vec3 rotX, rotY;
      orthonormalSystem(surfaceNormal, rotX, rotY);
      vec3 sampledDir = diffuseHemisphere(RND2, RND2);
      vec3 rotatedDir;
      rotatedDir.x = dot(vec3(rotX.x, rotY.x, surfaceNormal.x), sampledDir);
      rotatedDir.y = dot(vec3(rotX.y, rotY.y, surfaceNormal.y), sampledDir);
      rotatedDir.z = dot(vec3(rotX.z, rotY.z, surfaceNormal.z), sampledDir);
      float cost = dot(ray.direction, surfaceNormal);
      vec3 tmp(0,0,0);
      trace(ray, triangles, depth + 1, tmp);
      color = color + (tmp * triangles[intersection.triangleIndex].color) * cost * 0.1f * rrFactor;
  }

  if(strcmp(triangles[intersection.triangleIndex].material.type, "specular")){
    float cost = dot(ray.direction, surfaceNormal);
    ray.direction = ray.direction - normalize(surfaceNormal * (cost * 2));
    vec3 temp(0,0,0);
    trace(ray, triangles, depth + 1, temp);
    color = color + temp * rrFactor;
  }
}

void Draw(screen* screen, vector<Triangle>& triangles, Camera& cam, Light& light)
{
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  vec4 d;
  Intersection inter;
  vec3 color;
  Ray ray;

  vec3 **pixels = new vec3*[SCREEN_WIDTH];
  for(int i = 0; i < SCREEN_WIDTH; i++)
    pixels[i] = new vec3[SCREEN_HEIGHT];

  //#pragma omp parallel for private(inter, d, ray, color)
  for(int x = 0; x < SCREEN_WIDTH; x++){
    for(int y = 0; y < SCREEN_HEIGHT; y++){
      for(int s = 0; s < SAMPLES_PER_PIXEL; s++){
        vec3 color;
        Ray ray;
        d.x = (x - screen->width/2);
        d.y = (y - screen->height/2);
        d.z = cam.focalLength;
        ray.origin = cam.cameraPos;
        ray.direction = cam.cameraRotationX * cam.cameraRotationY * d;
        trace(ray, triangles, 0, color);
        pixels[x][y] = pixels[x][y] + color / SAMPLES_PER_PIXEL;
      }
    }
  }

  for(int x = 0; x < SCREEN_WIDTH; x++)
    for(int y = 0; y < SCREEN_HEIGHT; y++){
      if(pixels[x][y].x > 0 || pixels[x][y].y > 0 || pixels[x][y].z > 0)
        printf("%d %d: %f %f %f\n", x, y, pixels[x][y].x, pixels[x][y].y, pixels[x][y].z);
      PutPixelSDL(screen, x, y, pixels[x][y]);
    }

}

/*Place updates of parameters here*/
void Update(Camera &cam, Light &light)
{
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/

  handleKeyboard(cam, light);

}
