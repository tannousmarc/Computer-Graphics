#include <iostream>
#include <glm/glm.hpp>
#include "definitions.cpp"
#include <stdint.h>

SDL_Event event;

void TransformationMatrix(mat4x4 M){

}

void reset_camera(Camera &cam){
  cam.focalLength = SCREEN_HEIGHT;
  cam.cameraPos = vec4(0,0,-3.001,1);
  // I4
  cam.cameraRotation = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
  cam.yaw = 0.0;
  SDL_WarpMouseGlobal(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
}

void rotateY(Camera& cam, float angle){
  vec4 v1(cos(angle), 0, -sin(angle), 0);
  vec4 v2(0,1,0,0);
  vec4 v3(sin(angle), 0, cos(angle), 0);
  vec4 v4(0, 0, 0, 1);

  mat4 R(v1, v2, v3, v4);
  cam.cameraRotation = R;
}


void VertexShader(const vec4& v, ivec2& p, Camera cam){
  vec4 vNew = (v - cam.cameraPos) * cam.cameraRotation;
  p.x = (int) ((cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2));
  p.y = (int) ((cam.focalLength * vNew.y / vNew.z) + (SCREEN_HEIGHT / 2));
}

void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result){
  int N = result.size();
  vec2 step = vec2(b - a) / float(max(N - 1, 1));
  vec2 current(a);
  for(int i = 0; i < N; i++){
    result[i] = current;
    current += step;
  }
}

 void DrawLineSDL(screen* screen, ivec2 a, ivec2 b, vec3 color){
   ivec2 delta = glm::abs(a - b);
   int pixels = glm::max(delta.x, delta.y) + 1;
   vector<ivec2> line(pixels);
   Interpolate(a, b, line);
   for(int i = 0; i < line.size(); i++){
     if(line[i].x > 0 && line[i].x < SCREEN_WIDTH && line[i].y > 0 && line[i].y < SCREEN_HEIGHT)
     PutPixelSDL(screen, line[i].x, line[i].y, color);
   }
 }

 void DrawPolygonEdges(const vector<vec4>& vertices, Camera cam, screen* screen){
   int V = vertices.size();
   vector<ivec2> projectedVertices(V);
   for(int i = 0; i < V; i++){
      VertexShader(vertices[i], projectedVertices[i], cam);
   }
   for(int i = 0; i < V; i++){
     int j = (i + 1) % V;
     vec3 color(1, 1, 1);
     DrawLineSDL(screen, projectedVertices[i], projectedVertices[j], color);
   }
 }

int main( int argc, char* argv[] )
{
  
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  //SDL_ShowCursor(SDL_DISABLE);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  vector<Triangle> triangles;
  LoadTestModel(triangles);
  Camera cam;
  reset_camera(cam);
  while (NoQuitMessageSDL())
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
void Draw(screen* screen, vector<Triangle>& triangles, Camera cam)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  
  vec3 colour(1.0,0.0,0.0);
  for(uint32_t i = 0; i < triangles.size(); i++){
    vector<vec4> vertices(3);
    vertices[0] = triangles[i].v0;
    vertices[1] = triangles[i].v1;
    vertices[2] = triangles[i].v2;
    DrawPolygonEdges(vertices, cam, screen);
  }
}

/*Place updates of parameters here*/
void Update(Camera &cam)
{
  // static int t = SDL_GetTicks();
  // /* Compute frame time */
  // int t2 = SDL_GetTicks();
  // float dt = float(t2-t);
  // t = t2;
  // /*Good idea to remove this*/
  // std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/

  const uint8_t *keyState = SDL_GetKeyboardState(NULL);
  int lastRecordedX = 0;
  int lastRecordedY = 0;
  SDL_GetRelativeMouseState(&lastRecordedX, &lastRecordedY);
  if( keyState[SDL_SCANCODE_W] ){
    cam.cameraPos += cam.cameraRotation * vec4(0, 0, 0.02f, 0);
  }
  if( keyState[SDL_SCANCODE_S] ){
    cam.cameraPos -= cam.cameraRotation * vec4(0, 0, 0.02f, 0);
  }
  if( keyState[SDL_SCANCODE_A] ){
    cam.cameraPos -= cam.cameraRotation * vec4(0.02f, 0, 0, 0);
  }
  if( keyState[SDL_SCANCODE_D] ){
    cam.cameraPos += cam.cameraRotation * vec4(0.02f, 0, 0, 0);
  }

  if( lastRecordedX < 0){
    cam.yaw -= 0.02;
    rotateY(cam, cam.yaw);
  }
  if( lastRecordedX > 0){
    cam.yaw += 0.02;
    rotateY(cam, cam.yaw);
  }
  if( keyState[SDL_SCANCODE_SPACE]){
    reset_camera(cam);
  }
}
