#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <stdint.h>

using namespace std;
using glm::vec3;
using glm::mat3;

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define NUMBER_OF_STARS 2500
#define FULLSCREEN_MODE false
// #define VELOCITY -0.1


/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */
int t;
int focal = SCREEN_HEIGHT / 2;
vector<vec3> stars(NUMBER_OF_STARS);


/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw(screen* screen);

void Interpolate(float a, float b, vector<float>& result){
  if(result.size() == 1){
    result[0] = a;
    return;
  }
  float stepSize = (b - a) / ((float)result.size() - 1.0);
  result[0] = a;
  for(unsigned int i = 1; i < result.size(); i++){
    result[i] = result[i-1] + stepSize;
  }
}

void Interpolate(vec3 a, vec3 b, vector<vec3>& result){
  if(result.size() == 1){
    result[0] = vec3(a.x, a.y, a.z);
    return;
  }
  float redStepSize = (b.x - a.x) / ((float)result.size() - 1.0);
  float greenStepSize = (b.y - a.y) / ((float)result.size() - 1.0);
  float blueStepSize = (b.z - a.z) / ((float)result.size() - 1.0);
  result[0] = vec3(a.x, a.y, a.z);
  for(unsigned int i = 1; i < result.size(); i++){
    result[i] = vec3(result[i-1].x + redStepSize,
      result[i-1].y + greenStepSize,
      result[i-1].z + blueStepSize);
  }
}

int main( int argc, char* argv[] )
{
  for(int i = 0; i < NUMBER_OF_STARS; i++){
    float randomX = 2 * (float(rand()) / float(RAND_MAX)) - 1;
    float randomY = 2 * (float(rand()) / float(RAND_MAX)) - 1;
    float randomZ = float(rand()) / float(RAND_MAX);
    stars[i] = vec3(randomX, randomY, randomZ);
  }
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  t = SDL_GetTicks();	/*Set start value for timer.*/

  while( NoQuitMessageSDL() )
    {
      Draw(screen);
      Update();
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen)
{
  // vec3 topleft(1, 0, 0);
  // vec3 topRight(0, 0, 1);
  // vec3 bottomRight(0, 1, 0);
  // vec3 bottomLeft (1, 1, 0);
  //
  // vector<vec3> leftSide(SCREEN_HEIGHT);
  // vector<vec3> rightSide(SCREEN_HEIGHT);
  // Interpolate(topleft, bottomLeft, leftSide);
  // Interpolate(topRight, bottomRight, rightSide);

  // for(int j = 0; j < screen->height; j++){
  //   vector<vec3> temp(SCREEN_WIDTH);
  //   Interpolate(leftSide[j], rightSide[j], temp);
  //   for(int i = 0; i < screen->width; i++){
  //     PutPixelSDL(screen, i, j, temp[i]);
  //   }
  // }

  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  for(size_t s = 0; s < stars.size(); s++){
    int positionX = focal * (stars[s].x / stars[s].z) + SCREEN_WIDTH / 2;
    int positionY = focal * (stars[s].y / stars[s].z) + SCREEN_HEIGHT / 2;
    PutPixelSDL(screen, positionX, positionY, vec3(1, 1, 1));
  }

  // for(int i=0; i<1000; i++)
  //   {
  //     uint32_t x = rand() % screen->width;
  //     uint32_t y = rand() % screen->height;
  //     PutPixelSDL(screen, x, y, colourBlue);
  //   }
}

/*Place updates of parameters here*/
void Update()
{
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  for( unsigned int s=0; s<stars.size(); ++s ){
  // Add code for update of stars
    stars[s].z = (float)stars[s].z - 0.0005 * dt;
    if( stars[s].z <= 0 )
      stars[s].z += 1;
    if( stars[s].z > 1 )
      stars[s].z -= 1;
  }
  std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/
}
