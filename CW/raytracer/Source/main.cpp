/*
  Extensions:
              Kramer's rule
              OpenMP parallelization
              Path tracing:
                Anti aliasing
                Soft shadows
                Diffuse material
                Specular material
                Glass material (reflection + refraction)
                Mirrors
                Caustics
                Custom object loader
                Spheres
  TODO:
              Custom texture loader (DONE in rasterizer)
              Textured objects (DONE in rasterizer)
              Depth of field
              KD trees?
              Bidirectional path tracing?
*/

#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <random>

#include "definitions.h"
#include "light.cpp"
#include "intersections.cpp"
#include "camera.cpp"
#include "keyboard.cpp"
#include "utils.cpp"
#include "ray.h"
#include "pathtracer.cpp"
#include "sphere.cpp"
#include "parser.cpp"

int main(int argc, char* argv[])
{
  srand (time(NULL));

  vector<Triangle> triangles;
  vector<Sphere> spheres;
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
 
  Camera cam;
  reset_camera(cam);
  Light light;
  reset_light(light);

  vec3 **pixels = new vec3*[SCREEN_WIDTH];
  for(int i = 0; i < SCREEN_WIDTH; i++)
    pixels[i] = new vec3[SCREEN_HEIGHT];

  int samplesSeenSoFar;
  reset_evolutionModel(pixels, samplesSeenSoFar);

  // Load user parsed argument as scene
  if(argc == 2){
    if(strcmp(argv[1], "colorBleeding") == 0){
       LoadTestModelColorBleeding(triangles);
    }
    else if(strcmp(argv[1], "glassCube") == 0){
      light.lightPos = vec4(0.7, 0.95, -0.7, 1.0);
      light.lightColor = 14.0f * vec3(1, 1, 1);
      LoadTestModelGlassCube(triangles);
    }
    else if(strcmp(argv[1], "glassCubeSphere") == 0){
      light.lightPos = vec4(0.7, 0.95, -0.7, 1.0);
      light.lightColor = 14.0f * vec3(1, 1, 1);
      spheres.push_back(Sphere(0.3f, vec4(-0.45,0.7,-0.55,1), vec3(1,0,0), Material("glass")));
      LoadTestModelGlassCube(triangles);
    }
    else if(strcmp(argv[1], "glassWolf") == 0){
      LoadTestModelNone(triangles);
      light.lightPos = vec4(0.7, 0.95, -0.7, 1.0);
      light.lightColor = 14.0f * vec3(1, 1, 1);
      vector<Triangle> wolfTriangles;
      LoadObject("./Objects/wolf.obj", wolfTriangles);
      normaliseTriangles(wolfTriangles,
                        1.0,
                        -0.2, -1, -0.5,
                        3.15, -2.5, 0,
                        -1, 1, 1, "glass");
      triangles.insert( triangles.end(), wolfTriangles.begin(), wolfTriangles.end() );
    }
  }
  else{
     LoadTestModel(triangles);
  }

  while( NoQuitMessageSDL() )
    {
      Update(cam, light, pixels, samplesSeenSoFar);
      Draw(screen, triangles, spheres, cam, light, pixels, samplesSeenSoFar);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}


void Draw(screen* screen, vector<Triangle>& triangles,  vector<Sphere>& spheres, Camera& cam, Light& light, vec3** pixels, int& samplesSeenSoFar)
{
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  vec4 d;
  Intersection inter;
  vec3 color;
  Ray ray;

  samplesSeenSoFar++;

  #pragma omp parallel for private(inter, d, ray, color) schedule(dynamic)
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

      trace(ray, triangles, spheres, 0, color, light);
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
  std::cout << "Sample #" << samplesSeenSoFar << " rendered in: " << dt << " ms." << std::endl;

  handleKeyboard(cam, light, pixels, samplesSeenSoFar);
}
