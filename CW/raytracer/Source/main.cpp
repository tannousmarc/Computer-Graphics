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

float offsetX[5] = {0, 0, -0.5f, 0.5f, 0};
float offsetY[5] = {0, 0.5f, 0, 0, -0.5f};
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
      // incremeneted each time we find a neighbour inside the image boundaries
      int neighbourCounter = 0;
      vec3 finalColor(0.f,0.f,0.f);
      // compute values in 5 positions: above, below, left, right, center.
      for(int neighbourIndex = 0; ANTIALIASING_MODE ? neighbourIndex < 5 : neighbourIndex < 1; neighbourIndex++){
        d.x = (x - screen->width/2 + offsetX[neighbourIndex]);
        d.y = (y - screen->height/2 + offsetY[neighbourIndex]);
        d.z = cam.focalLength;
        if(x + offsetX[neighbourIndex] >= 0 && x + offsetX[neighbourIndex]
           < SCREEN_WIDTH && y + offsetY[neighbourIndex] >= 0 &&
           y + offsetY[neighbourIndex] < SCREEN_HEIGHT){
          neighbourCounter++;
          if(ClosestIntersection(cam,
                                 cam.cameraRotationX * cam.cameraRotationY * d,
                                 triangles, inter)){
            vec3 lightD = directLight(cam, triangles, inter, light);
            color = triangles[inter.triangleIndex].color;

            if(triangles[inter.triangleIndex].isMirror){
              color = mirror(cam, triangles, inter, light, normalize(cam.cameraRotationX * cam.cameraRotationY * d), 0);
            }
            finalColor += color * lightD;
          }
        }
      }
      finalColor.x /= neighbourCounter;
      finalColor.y /= neighbourCounter;
      finalColor.z /= neighbourCounter;
      PutPixelSDL(screen, x, y, finalColor);
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
