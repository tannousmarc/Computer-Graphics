#include "keyboard.h"
#include "definitions.h"
#include "utils.h"

void handleKeyboard(Camera &cam, Light &light, vec3** pixels, int& samplesSeenSoFar){
  const uint8_t *keyState = SDL_GetKeyboardState(NULL);

  if( keyState[SDL_SCANCODE_W] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    cam.cameraPos += cam.cameraRotationX * cam.cameraRotationY * vec4(0, 0, 0.25f, 0);
  }
  if( keyState[SDL_SCANCODE_S] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    cam.cameraPos -= cam.cameraRotationX * cam.cameraRotationY * vec4(0, 0, 0.25f, 0);
  }
  if( keyState[SDL_SCANCODE_A] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    cam.cameraPos -= cam.cameraRotationX * cam.cameraRotationY * vec4(0.25f, 0, 0, 0);
  }
  if( keyState[SDL_SCANCODE_D] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    cam.cameraPos += cam.cameraRotationX * cam.cameraRotationY * vec4(0.25f, 0, 0, 0);
  }

  if( keyState[SDL_SCANCODE_UP] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    light.lightPos += vec4(0, 0, 0.02f, 0);
  }
  if( keyState[SDL_SCANCODE_DOWN] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    light.lightPos -= vec4(0, 0, 0.02f, 0);
  }
  if( keyState[SDL_SCANCODE_LEFT] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    light.lightPos -= vec4(0.02f, 0, 0, 0);
  }
  if( keyState[SDL_SCANCODE_RIGHT] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    light.lightPos += vec4(0.02f, 0, 0, 0);
  }
  if( keyState[SDL_SCANCODE_RIGHTBRACKET] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    light.lightPos -= vec4(0, 0.02f, 0, 0);
  }
  if( keyState[SDL_SCANCODE_BACKSLASH] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    light.lightPos += vec4(0, 0.02f, 0, 0);
  }

  if( keyState[SDL_SCANCODE_J] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    cam.yaw -= 0.04;
    rotateY(cam, cam.yaw);
  }
  if( keyState[SDL_SCANCODE_L] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    cam.yaw += 0.04;
    rotateY(cam, cam.yaw);
  }
  if( keyState[SDL_SCANCODE_I] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    cam.pitch += 0.04;
    rotateX(cam, cam.pitch);
  }
  if( keyState[SDL_SCANCODE_K] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    cam.pitch -= 0.04;
    rotateX(cam, cam.pitch);
  }

  if( keyState[SDL_SCANCODE_SPACE] ){
    reset_evolutionModel(pixels, samplesSeenSoFar);
    reset_camera(cam);
    reset_light(light);
  }
}
