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
std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0, 1);
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

void trace(Ray &ray, const vector<Triangle>& triangles, int depth, vec3& color, Light light){
  Intersection intersection;
  if(closestIntersection(ray, triangles, intersection) == false)
    return;

  const float rrStopProbability = 0.9f;
  if(depth >= 5 || distribution(generator) > rrStopProbability){
      color = triangles[intersection.triangleIndex].color * directLight(triangles, intersection, light);
      return;
  }

  vec4 surfaceNormal = triangles[intersection.triangleIndex].normal;
  ray.origin = intersection.position;

  if(strcmp(triangles[intersection.triangleIndex].material.type, "diffuse") == 0){
      vec3 rotX, rotY;
      orthonormalSystem(surfaceNormal, rotX, rotY);
      vec3 sampledDir = diffuseHemisphere(distribution(generator), distribution(generator));
      vec3 rotatedDir(sampledDir.x * rotY.x + sampledDir.y * surfaceNormal.x + sampledDir.z * rotX.x,
                      sampledDir.x * rotY.y + sampledDir.y * surfaceNormal.y + sampledDir.z * rotX.y,
                      sampledDir.x * rotY.z + sampledDir.y * surfaceNormal.z + sampledDir.z * rotX.z);

      ray.origin = ray.origin + vec4(rotatedDir, 1) * 0.001f;
      ray.direction = vec4(rotatedDir, 1);
      float cost = dot(ray.direction, surfaceNormal);
      vec3 tmp(0,0,0);

      trace(ray, triangles, depth + 1, tmp, light);

      color = color + (tmp * triangles[intersection.triangleIndex].color);
  }

  // if(strcmp(triangles[intersection.triangleIndex].material.type, "specular")){
  //   float cost = dot(ray.direction, surfaceNormal);
  //   ray.direction = ray.direction - normalize(surfaceNormal * (cost * 2));
  //   vec3 temp(0,0,0);
  //   trace(ray, triangles, depth + 1, temp, light);
  //   color = color + temp * rrFactor;
  // }
}

void Draw(screen* screen, vector<Triangle>& triangles, Camera& cam, Light& light)
{
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  vec4 d;
  Intersection inter;
  vec3 color;
  Ray ray;

  #pragma omp parallel for private(inter, d, ray, color)
  for(int x = 0; x < SCREEN_WIDTH; x++){
    for(int y = 0; y < SCREEN_HEIGHT; y++){
      vec3 pixel(0,0,0);
      for(int s = 0; s < SAMPLES_PER_PIXEL; s++){
        vec3 color(0,0,0);
        Ray ray;
        d.x = (x - screen->width/2) + (distribution(generator) * 2 - 1) * 0.1;
        d.y = (y - screen->height/2) + (distribution(generator) * 2 - 1) * 0.1;
        d.z = cam.focalLength;
        ray.origin = cam.cameraPos;
        ray.direction = cam.cameraRotationX * cam.cameraRotationY * d;
        trace(ray, triangles, 0, color, light);
        pixel += color;
      }
      PutPixelSDL(screen, x, y, pixel / (SAMPLES_PER_PIXEL * (1 / (2 * 3.1415f))));
    }
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
