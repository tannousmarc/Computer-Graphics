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

  vec3 **pixels = new vec3*[SCREEN_WIDTH];
  for(int i = 0; i < SCREEN_WIDTH; i++)
    pixels[i] = new vec3[SCREEN_HEIGHT];

  int samplesSeenSoFar;
  reset_evolutionModel(pixels, samplesSeenSoFar);

  while( NoQuitMessageSDL() )
    {
      Update(cam, light, pixels, samplesSeenSoFar);
      Draw(screen, triangles, cam, light, pixels, samplesSeenSoFar);
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

  const float russianRoulette = 0.85f;
  if (depth >= 5 || distribution(generator) > russianRoulette) {
      Light softShadowLight;
      softShadowLight.lightPos.x = light.lightPos.x + (distribution(generator) * 2 - 1) * 0.1f;
      softShadowLight.lightPos.y = light.lightPos.y;
      softShadowLight.lightPos.z = light.lightPos.z + (distribution(generator) * 2 - 1) * 0.1f;
      softShadowLight.lightPos.w = light.lightPos.w;
      softShadowLight.lightColor = light.lightColor;
      color = triangles[intersection.triangleIndex].color * directLight(triangles, intersection, softShadowLight);
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

      ray.direction = normalize(vec4(rotatedDir, 1));
      ray.origin = ray.origin + ray.direction * 0.001f;

      vec3 tmp(0,0,0);

      trace(ray, triangles, depth + 1, tmp, light);

      color = color + (tmp * triangles[intersection.triangleIndex].color) * dot(ray.direction, surfaceNormal);
  }

  else if(strcmp(triangles[intersection.triangleIndex].material.type, "glossy") == 0){
    float currRefraction = INDEX_OF_REFRACTION;
		float R0 = (1.0 - currRefraction) / (1.0 + currRefraction);
		R0 = R0 * R0;

    // inside
    if(dot(surfaceNormal, ray.direction) > 0){
      surfaceNormal = surfaceNormal * -1.0f;
      currRefraction = 1 / currRefraction;
    }
		currRefraction = 1 / currRefraction;

    float cost1 = dot(surfaceNormal, ray.direction) * -1.0f;
    float cost2 = 1.0f - currRefraction * currRefraction * (1.0f - cost1 * cost1);
    // Schlick approx
    float Rprob = R0 + (1.0f - R0) * pow(1.0f - cost1, 5.0f);
    ray.direction = normalize(ray.direction + surfaceNormal * (cost1 * 2.0f));
    // refraction direction
		if (cost2 > 0 && distribution(generator) > Rprob) {
			ray.direction = normalize((ray.direction * currRefraction) + (surfaceNormal * (currRefraction * cost1 - sqrt(cost2))));
		}
    // reflection direction
		else {
			ray.direction = normalize(ray.direction + surfaceNormal * (cost1 * 2.0f));
		}

		vec3 tmp(0.f, 0.f, 0.f);
		trace(ray, triangles, depth + 1, tmp, light);
		color = color + tmp;
	}

  // else if(strcmp(triangles[intersection.triangleIndex].material.type, "specular") == 0){
  //   ray.direction =  normalize(ray.direction - surfaceNormal * (dot(ray.direction, surfaceNormal) * 2.f));
  //   vec3 temp(0,0,0);
  //   trace(ray, triangles, depth + 1, temp, light);
  //   color = color + temp;
  // }
}

void Draw(screen* screen, vector<Triangle>& triangles, Camera& cam, Light& light, vec3** pixels, int& samplesSeenSoFar)
{
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  vec4 d;
  Intersection inter;
  vec3 color;
  Ray ray;

  samplesSeenSoFar++;

  #pragma omp parallel for private(inter, d, ray, color)
  for(int x = 0; x < SCREEN_WIDTH; x++){
    for(int y = 0; y < SCREEN_HEIGHT; y++){
      vec3 pixel(0,0,0);
      vec3 color(0,0,0);
      Ray ray;
      d.x = (x - screen->width/2) + (distribution(generator) * 2 - 1) * 0.1;
      d.y = (y - screen->height/2) + (distribution(generator) * 2 - 1) * 0.1;
      d.z = cam.focalLength;
      ray.origin = cam.cameraPos;
      ray.direction = cam.cameraRotationX * cam.cameraRotationY * d;
      trace(ray, triangles, 0, color, light);

      // TODO: This could overflow really fast
      pixels[x][y] += color;
    }
  }

  for(int x = 0; x < SCREEN_WIDTH; x++){
    for(int y = 0; y < SCREEN_HEIGHT; y++){
        PutPixelSDL(screen, x, y, pixels[x][y] / (samplesSeenSoFar * (1 / (2 * 3.1415f))));
    }
  }




}

/*Place updates of parameters here*/
void Update(Camera &cam, Light &light, vec3** pixels, int& samplesSeenSoFar)
{
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  std::cout << "Sample #" << samplesSeenSoFar << " rendered in: " << dt << " ms." << std::endl;
  /* Update variables*/

  handleKeyboard(cam, light, pixels, samplesSeenSoFar);

}
