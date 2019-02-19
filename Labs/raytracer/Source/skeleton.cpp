#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>  
#include <limits.h>
#include "intersections.cpp"

//Extensions: Kramer's rule,

vec3 indirectLight = INDIRECT_LIGHT_FACTOR * vec3(1,1,1);

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

vec3 directLight(Camera& cam, const vector<Triangle>& triangles, Intersection& inter, Light light){
  Intersection light_inter = inter;
  vec4 vecr = normalize(light.lightPos - inter.position);
  float r = glm::length(light.lightPos - inter.position);

  vec3 d = (light.lightColor * max(dot(vecr, normalize(triangles[inter.triangleIndex].normal)), 0.f))
           /
           (float)(4.f * 3.14159 * r * r);

  light_inter.distance = r;
  if(ClosestIntersectionLight(inter.position + vecr * 0.001f, vecr, triangles, light_inter)){
    d = vec3(0.f, 0.f, 0.f);
  }
  return d + indirectLight;
}

void rotateY(Camera& cam, float angle){
  vec4 v1(cos(angle), 0, -sin(angle), 0);
  vec4 v2(0,1,0,0);
  vec4 v3(sin(angle), 0, cos(angle), 0);
  vec4 v4(0, 0, 0, 1);

  mat4 R(v1, v2, v3, v4);
  cam.cameraRotation = R;
}

int main( int argc, char* argv[] )
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
      Update(cam);
      Draw(screen, triangles, cam, light);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

float dirX[] = {-0.5, 0, 0.5, 0};
float dirY[] = {0,  0.5, 0, -0.5};

void Draw(screen* screen, vector<Triangle>& triangles, Camera& cam, Light& light)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  vec3 colour(1.0,0.0,0.0);
  vec4 d;

  if (ANTI_ALIASING_MODE){
    for(int y=0; y<screen->height; y++){
      for(int x=0; x<screen->width; x++){
        vec3 neighbouringPixelColours[4];
        int numberOfNeighbours = 0;
        for(int n = 0; n < 4; n++){
          neighbouringPixelColours[n] = vec3(0.0,0.0,0.0);
          if(x + dirX[n] < screen->width && x + dirX[n] >= 0
            && x + dirY[n] < screen->height && x + dirY[n] >= 0){
              numberOfNeighbours++;
              Intersection inter;
              d.x = (x + dirX[n] - screen->width/2);
              d.y = (y + dirY[n] - screen->height/2);
              d.z = cam.focalLength;
              if(ClosestIntersection(cam, cam.cameraRotation*d, triangles, inter)){
                vec3 lightD = directLight(cam, triangles, inter, light);
                for(int i = -50; i < 50; i++){
                  Light newLight;
                  newLight.lightPos = vec4(light.lightPos.x + i * 0.001, light.lightPos.y, light.lightPos.z + i * 0.001, 1.0);
                  newLight.lightColor = light.lightColor;
                  lightD += directLight(cam, triangles, inter, newLight);
                }

                lightD.x /= 101;
                lightD.y /= 101;
                lightD.z /= 101;
                neighbouringPixelColours[n] = triangles[inter.triangleIndex].color * lightD;
              }
            }

        }

        Intersection inter;
        d.x = (x - screen->width/2);
        d.y = (y - screen->height/2);
        d.z = cam.focalLength;

        if(ClosestIntersection(cam, cam.cameraRotation*d, triangles, inter)){
          vec3 lightD = directLight(cam, triangles, inter, light);
          vec3 neighbouringSum = neighbouringPixelColours[0] + neighbouringPixelColours[1] + neighbouringPixelColours[2] + neighbouringPixelColours[3];
          vec3 result = (neighbouringSum + triangles[inter.triangleIndex].color * lightD);
          result.x /= (numberOfNeighbours + 1);
          result.y /= (numberOfNeighbours + 1);
          result.z /= (numberOfNeighbours + 1);
          PutPixelSDL(screen, x, y, result);
        }
      }
    }
  }
  else{
    for(int y=0; y<screen->height; y++){
      for(int x=0; x<screen->width; x++){
        Intersection inter;
        d.x = (x - screen->width/2);
        d.y = (y - screen->height/2);
        d.z = cam.focalLength;

        if(ClosestIntersection(cam, cam.cameraRotation*d, triangles, inter)){
          vec3 lightD = directLight(cam, triangles, inter, light);
          PutPixelSDL(screen, x, y, triangles[inter.triangleIndex].color * lightD);
        }
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

  const uint8_t *keyState = SDL_GetKeyboardState(NULL);

  if( keyState[SDL_SCANCODE_UP] ){
    cam.cameraPos += cam.cameraRotation * vec4(0, 0, 0.25f, 0);
  }
  if( keyState[SDL_SCANCODE_DOWN] ){
    cam.cameraPos -= cam.cameraRotation * vec4(0, 0, 0.25f, 0);
  }
  if( keyState[SDL_SCANCODE_LEFT] ){
    cam.cameraPos -= cam.cameraRotation * vec4(0.25f, 0, 0, 0);
  }
  if( keyState[SDL_SCANCODE_RIGHT] ){
    cam.cameraPos += cam.cameraRotation * vec4(0.25f, 0, 0, 0);
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
