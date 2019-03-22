#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include "definitions.h"
#include "light.cpp"
#include "intersections.cpp"
#include "mirror.cpp"
#include "camera.cpp"
#include "keyboard.cpp"

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

void Draw(screen* screen, vector<Triangle>& triangles, Camera& cam, Light& light)
{
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  vec4 d;
  Intersection inter;
  vec3 color;

  // add values here for aliasing
  #pragma omp parallel for private(inter, d)
  for(int y=0; y<screen->height; y++){
      for(int x=0; x<screen->width; x++){

        d.x = (x - screen->width/2);
        d.y = (y - screen->height/2);
        d.z = cam.focalLength;

        if(ClosestIntersection(cam, cam.cameraRotation*d, triangles, inter)){
          vec3 lightD = directLight(cam, triangles, inter, light);
          color = triangles[inter.triangleIndex].color;
               if(triangles[inter.triangleIndex].isMirror){
                 color = mirror(cam, triangles, inter, light, cam.cameraRotation * d, 0);
               }

          PutPixelSDL(screen, x, y, color * lightD);
        }
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
