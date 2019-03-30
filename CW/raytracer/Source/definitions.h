#ifndef DEFINITIONS
#define DEFINITIONS


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
  mat4 cameraRotationX;
  mat4 cameraRotationY;
  float yaw;
  float pitch;
};

struct Intersection
{
  vec4 position;
  float distance;
  vec3 color;
  vec4 normal;
  Material material;
};

struct Light{
  vec4 lightPos;
  vec3 lightColor;
};

// Helpers for random generation
std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0, 1);

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1280
#define FULLSCREEN_MODE true

#define MAX_DEPTH 20

#define EPSILON 1e-6
#define GLASS_INDEX_OF_REFRACTION 1.512f
#define AIR_INDEX_OF_REFRACTION 1.000f

#include "sphere.h"

void Update(Camera &cam, Light &light, vec3** pixels, int& samplesSeenSoFar);

void Draw(screen* screen, vector<Triangle>& triangles, vector<Sphere>& spheres, Camera& cam, Light& light, vec3** pixels, int& samplesSeenSoFar);

#endif
