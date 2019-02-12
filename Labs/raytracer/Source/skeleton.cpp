#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <limits.h>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;


#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 360
#define FULLSCREEN_MODE false

// struct Camera{
//   float focalLength;
//   vec4 cameraPos;
// }

struct Camera{
  float focalLength;
  vec4 cameraPos;
  mat4 cameraRotation;
  float yaw;
};

struct Intersection
{
  vec4 position;
  float distance;
  int triangleIndex;
};

struct Light{
  vec4 lightPos;
  vec3 lightColor;
}



/* ----------------------------------------------------------------------------*/


/*

TODO Points for triangles in 3D TODO

Axes:
  e1 = v1 - v0
  e2 = v2 - v0

Point r=(u, v) in triangle plane:
  r = v0 + ue1 + ve2

Points within triangle (and plane):
  0 < u
  0 < v
  u + v < 1

TODO Rays in 3D TODO
start position s element of R4
and direction d element of R4

r = s + td

t element of R = position on the line, signed distance

0 <= t


TODO Intersection TODO

Intersection point x = A^(-1)b
A = (-d  e1  e2)
b = s - v0
x = (t  u  v)^T

 */
/* FUNCTIONS                                                                   */



void Update(Camera& cam);
void Draw(screen* screen);

void reset_camera(Camera &cam){
  cam.focalLength = SCREEN_WIDTH;
  cam.cameraPos = vec4(0,0,-4,1);
  // I4
  cam.cameraRotation = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
  cam.yaw = 0.0;
}

void reset_light(Light &light){
  light.lightPos = vec4(0, -0.5, -0.7, 1.0);
  light.lightColor = 14.0f * vec3(1, 1, 1);
}

void rotateY(Camera& cam, float angle){
  vec4 v1(cos(angle), 0, -sin(angle), 0);
  vec4 v2(0,1,0,0);
  vec4 v3(sin(angle), 0, cos(angle), 0);
  vec4 v4(0, 0, 0, 1);

  mat4 R(v1, v2, v3, v4);
  cam.cameraRotation = R;
}


bool ClosestIntersection( Camera& cam, vec4 dir,
    const vector<Triangle>& triangles,
    Intersection& closestIntersection ){

  float minimumDistance = std::numeric_limits<float>::max();
  bool okay = false;

  for(unsigned int i = 0; i < triangles.size(); i++){
    Triangle triangle = triangles[i];
    vec4 v0 = triangle.v0;
    vec4 v1 = triangle.v1;
    vec4 v2 = triangle.v2;
    vec3 e1 = vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
    vec3 e2 = vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
    vec3 b = vec3(cam.cameraPos.x-v0.x,cam.cameraPos.y-v0.y,cam.cameraPos.z-v0.z);
    mat3 A( vec3(-dir), e1, e2 );
    // EXTENSION
    mat3 m = A;
    m[0] = b;
    float t = glm::determinant(m) / glm::determinant(A);
    if(t > 0 && t < minimumDistance){
      vec3 x = glm::inverse( A ) * b;
      if(x.y >= 0 && x.z >= 0 && (x.y + x.z <= 1)){
        minimumDistance = abs(x.x);
        closestIntersection.position = v0 + x.y*vec4(e1.x, e1.y, e1.z, 0) + x.z*vec4(e2.x, e2.y, e2.z, 0);
        closestIntersection.distance = abs(x.x);
        closestIntersection.triangleIndex = i;
        okay = true;
      }
    }
    // vec3 x = glm::inverse( A ) * b;
  }

  return okay;
};

void Draw(screen* screen, vector<Triangle>& triangles, Camera& cam);

int main( int argc, char* argv[] )
{
  vector<Triangle> triangles;
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  LoadTestModel(triangles);
  Camera cam;
  reset_camera(cam);

  while( NoQuitMessageSDL() )
    {
      Update(cam);
      Draw(screen, triangles, cam);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen, vector<Triangle>& triangles, Camera& cam)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  vec3 colour(1.0,0.0,0.0);
  vec4 d;

  // for(int i=0; i<1000; i++)
  //   {
  //     uint32_t x = rand() % screen->width;
  //     uint32_t y = rand() % screen->height;
  //     PutPixelSDL(screen, x, y, colour);
  //   }
  for(int y=0; y<screen->height; y++){
    for(int x=0; x<screen->width; x++){
      Intersection inter;
      d.x = (x - screen->width/2);
      d.y = (y - screen->height/2);
      d.z = cam.focalLength;

      if(ClosestIntersection(cam, cam.cameraRotation*d, triangles, inter)){
        PutPixelSDL(screen, x, y, triangles[inter.triangleIndex].color);
      }
    }
  }
}

/*Place updates of parameters here*/
void Update(Camera &cam)
{
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/

  // SDL_Event e;
  const uint8_t *keyState = SDL_GetKeyboardState(NULL);
  if( keyState[SDL_SCANCODE_UP] ){
    cam.cameraPos += cam.cameraRotation * vec4(0, 0, 0.25f, 0);
  }
  if( keyState[SDL_SCANCODE_DOWN] ){
    cam.cameraPos -= cam.cameraRotation * vec4(0, 0, 0.25f, 0);
  }
  if( keyState[SDL_SCANCODE_LEFT] ){
    // cameraPos.x -=0.25;
    cam.cameraPos -= cam.cameraRotation * vec4(0.25f, 0, 0, 0);
    // cam.cameraPos.x -= 0.25;
  // Move camera to the left
  }
  if( keyState[SDL_SCANCODE_RIGHT] ){
    // cameraPos.x +=0.25;
    cam.cameraPos += cam.cameraRotation * vec4(0.25f, 0, 0, 0);
  // Move camera to the right
  }
  if( keyState[SDL_SCANCODE_Q]){
    cam.yaw -= 0.04;
    rotateY(cam, cam.yaw);
  }
  if( keyState[SDL_SCANCODE_E]){
    cam.yaw += 0.04;
    rotateY(cam, cam.yaw);
  }
  if( keyState[SDL_SCANCODE_SPACE]){
    reset_camera(cam);
  }
}
