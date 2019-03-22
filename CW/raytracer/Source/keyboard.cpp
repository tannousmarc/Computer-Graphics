#include "keyboard.h"
#include "definitions.h"

void handleKeyboard(Camera &cam, Light &light){
  const uint8_t *keyState = SDL_GetKeyboardState(NULL);

  if( keyState[SDL_SCANCODE_W] ){
    cam.cameraPos += cam.cameraRotationX * cam.cameraRotationY * vec4(0, 0, 0.25f, 0);
  }
  if( keyState[SDL_SCANCODE_S] ){
    cam.cameraPos -= cam.cameraRotationX * cam.cameraRotationY * vec4(0, 0, 0.25f, 0);
  }
  if( keyState[SDL_SCANCODE_A] ){
    cam.cameraPos -= cam.cameraRotationX * cam.cameraRotationY * vec4(0.25f, 0, 0, 0);
  }
  if( keyState[SDL_SCANCODE_D] ){
    cam.cameraPos += cam.cameraRotationX * cam.cameraRotationY * vec4(0.25f, 0, 0, 0);
  }

  if( keyState[SDL_SCANCODE_UP] ){
    light.lightPos += vec4(0, 0, 0.02f, 0);
  }
  if( keyState[SDL_SCANCODE_DOWN] ){
    light.lightPos -= vec4(0, 0, 0.02f, 0);
  }
  if( keyState[SDL_SCANCODE_LEFT] ){
    light.lightPos -= vec4(0.02f, 0, 0, 0);
  }
  if( keyState[SDL_SCANCODE_RIGHT] ){
    light.lightPos += vec4(0.02f, 0, 0, 0);
  }
  if( keyState[SDL_SCANCODE_RIGHTBRACKET] ){
    light.lightPos -= vec4(0, 0.02f, 0, 0);
  }
  if( keyState[SDL_SCANCODE_BACKSLASH] ){
    light.lightPos += vec4(0, 0.02f, 0, 0);
  }

  if( keyState[SDL_SCANCODE_J]){
    cam.yaw -= 0.04;
    rotateY(cam, cam.yaw);
  }
  if( keyState[SDL_SCANCODE_L]){
    cam.yaw += 0.04;
    rotateY(cam, cam.yaw);
  }
  if( keyState[SDL_SCANCODE_I]){
    cam.pitch += 0.04;
    rotateX(cam, cam.pitch);
  }
  if( keyState[SDL_SCANCODE_K]){
    cam.pitch -= 0.04;
    rotateX(cam, cam.pitch);
  }

  if( keyState[SDL_SCANCODE_SPACE]){
    reset_camera(cam);
    reset_light(light);
  }
}
