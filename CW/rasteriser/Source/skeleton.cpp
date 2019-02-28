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

void VertexShader(const vec4& v, Pixel& p, Camera cam){
  vec4 vNew = (v - cam.cameraPos) * cam.cameraRotation;
  p.x = (int) ((cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2));
  p.y = (int) ((cam.focalLength * vNew.y / vNew.z) + (SCREEN_HEIGHT / 2));
  p.zinv = 1 / (glm::distance(cam.cameraPos, v));
}

void Interpolate(Pixel a, Pixel b, vector<Pixel>& result){
  int N = result.size();
  ivec2 ai = ivec2(a.x, a.y);
  ivec2 bi = ivec2(b.x, b.y);
  vec2 step = vec2(bi - ai) / float(max(N - 1, 1));
  float stepz = (b.zinv - a.zinv) / float(max(N - 1, 1));
  vec2 current(ai);
  float currentInvZ = a.zinv;
  for(int i = 0; i < N; i++){
    result[i].x = current.x;
    result[i].y = current.y;
    result[i].zinv = currentInvZ;
    current += step;
    currentInvZ += stepz;
  }
}

 void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels){
   int largestY = -numeric_limits<int>::max(), smallestY = numeric_limits<int>::max();
   for(int i = 0; i < vertexPixels.size(); i++){
     if(vertexPixels[i].y > largestY){
       largestY = vertexPixels[i].y;
     }
     if(vertexPixels[i].y < smallestY){
       smallestY = vertexPixels[i].y;
     }
   }
   int rows = largestY - smallestY + 1;
   leftPixels.resize(rows);
   rightPixels.resize(rows);
   for(int i = 0; i < rows; i++){
     leftPixels[i].x = numeric_limits<int>::max();
     rightPixels[i].x = -numeric_limits<int>::max();
   }

   for(int i = 0; i < vertexPixels.size(); i++){
     for(int j = i + 1; j < vertexPixels.size(); j++){
       ivec2 ii = ivec2(vertexPixels[i].x, vertexPixels[i].y);
       ivec2 ij = ivec2(vertexPixels[j].x, vertexPixels[j].y);
       ivec2 delta = glm::abs(ii - ij);
       int pixels = glm::max(delta.x, delta.y) + 1;
       vector<Pixel> line(pixels);
       Interpolate(vertexPixels[i], vertexPixels[j], line);
       for(int k = 0; k < line.size(); k++){
         
         int row = line[k].y - smallestY;
         //printf("Setting row: %d rows: %d \n", row, rows);
         if(row >= 0){
          if(line[k].x < leftPixels[row].x){
            leftPixels[row].x = line[k].x;
            leftPixels[row].y = line[k].y;
            leftPixels[row].zinv = line[k].zinv;
          }
          if(line[k].x > rightPixels[row].x){
            rightPixels[row].x = line[k].x;
            rightPixels[row].y = line[k].y;
            leftPixels[row].zinv = line[k].zinv;
          }
        }
       }
     }
   }
 }

 void DrawPolygonRows(screen *screen, const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels, vec3 color, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH] ){
   for(int i = 0; i < leftPixels.size(); i++){
     ivec2 ii = ivec2(leftPixels[i].x, leftPixels[i].y);
     ivec2 ij = ivec2(rightPixels[i].x, rightPixels[i].y);
     ivec2 delta = glm::abs(ii - ij);
     int pixels = glm::max(delta.x, delta.y) + 1;
     vector<Pixel> zinvs(pixels);
     Interpolate(leftPixels[i], rightPixels[i], zinvs);
     int current = 0;
     for(int j = leftPixels[i].x; j <= rightPixels[i].x; j++){
       if(j > 0 && j < SCREEN_WIDTH && leftPixels[i].y > 0 && leftPixels[i].y < SCREEN_HEIGHT &&
          zinvs[current].zinv > depthBuffer[leftPixels[i].y][j]){
        PutPixelSDL(screen, j, leftPixels[i].y, color);
        depthBuffer[leftPixels[i].y][j] = zinvs[current].zinv;
       }
       current++;
     }
   }
 }

 void DrawPolygon(screen *screen, const vector<vec4>& vertices, Camera cam, vec3 color, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH]){
   int V = vertices.size();

   vector<Pixel> vertexPixels(V);
   for(int i = 0; i < V; i++){
     VertexShader(vertices[i], vertexPixels[i], cam);
   }
   vector<Pixel> leftPixels;
   vector<Pixel> rightPixels;
   ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
   DrawPolygonRows(screen, leftPixels, rightPixels, color, depthBuffer);
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
  float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
  for(int y = 0; y < SCREEN_HEIGHT; y++){
    for(int x = 0; x < SCREEN_WIDTH; x++){
      depthBuffer[y][x] = 0.0f;
    }
  }


  for(uint32_t i = 0; i < triangles.size(); i++){
    vector<vec4> vertices(3);
    vertices[0] = triangles[i].v0;
    vertices[1] = triangles[i].v1;
    vertices[2] = triangles[i].v2;
    DrawPolygon(screen, vertices, cam, triangles[i].color, depthBuffer);
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
