#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"

using namespace std;
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;
using glm::mat4x4;
using glm::ivec2;

struct Camera{
  float focalLength;
  vec4 cameraPos;
  mat4 cameraRotation;
  float yaw;
  float xaw;
};

struct Pixel{
  int x;
  int y;
  float zinv;
};

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FULLSCREEN_MODE true

void Update(Camera &cam);
void Draw(screen* screen, vector<Triangle>& triangles, Camera cam);

// struct Intersection
// {
//   vec4 position;
//   float distance;
//   int triangleIndex;
// };

// struct Light{
//   vec4 lightPos;
//   vec3 lightColor;
// };

// #define SCREEN_WIDTH 1024
// #define SCREEN_HEIGHT 1024
// #define FULLSCREEN_MODE true

// #define SOFT_SHADOW_DISPLACEMENT 0
// #define ANTI_ALIASING_MODE true

// #define INDIRECT_LIGHT_FACTOR 0.25f

// void Update(Camera& cam);

// void Draw(screen* screen, vector<Triangle>& triangles, Camera& cam, Light& light);