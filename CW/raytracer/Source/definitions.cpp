#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;

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
};

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 320
#define FULLSCREEN_MODE true

#define SOFT_SHADOW_DISPLACEMENT 0
#define ANTI_ALIASING_MODE false

#define INDIRECT_LIGHT_FACTOR 0.25f

void Update(Camera& cam);

void Draw(screen* screen, vector<Triangle>& triangles, Camera& cam, Light& light);