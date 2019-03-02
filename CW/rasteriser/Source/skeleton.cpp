#include <iostream>
#include <glm/glm.hpp>
#include "definitions.cpp"
#include <stdint.h>

SDL_Event event;

void reset_camera(Camera &cam){
  cam.focalLength = SCREEN_HEIGHT;
  cam.cameraPos = vec4(0,0,-3.001,1);
  // I4
  cam.cameraRotationY = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
  cam.cameraRotationX = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
  cam.cameraRotationZ = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
  cam.yaw = 0.0;
  cam.pitch = 0.0;
  SDL_WarpMouseGlobal(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
}

void reset_light(Light &light){
  light.lightPos = vec3(0, -0.5f, -0.7f);
  light.lightPower = 14.0f*vec3(1, 1, 1);
  light.indirectLightPowerPerArea = 0.5f*vec3(1, 1, 1);
}

void rotateY(Camera& cam, float angle){
  vec4 v1(cos(angle), 0, -sin(angle), 0);
  vec4 v2(0,1,0,0);
  vec4 v3(sin(angle), 0, cos(angle), 0);
  vec4 v4(0, 0, 0, 1);

  mat4 R(v1, v2, v3, v4);
  cam.cameraRotationY = R;
}

void rotateX(Camera& cam, float angle){
  vec4 v1(1, 0, 0, 0);
  vec4 v2(0, cos(angle), sin(angle),0);
  vec4 v3(0, -sin(angle), cos(angle), 0);
  vec4 v4(0, 0, 0, 1);

  mat4 R(v1, v2, v3, v4);
  cam.cameraRotationX = R;
}

void VertexShader(const Vertex& v, Pixel& p, Camera cam){
  vec4 vNew = (v.position - cam.cameraPos) * (cam.cameraRotationX * cam.cameraRotationY * cam.cameraRotationZ);
  p.x = (int) ((cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2));
  p.y = (int) ((cam.focalLength * vNew.y / vNew.z) + (SCREEN_HEIGHT / 2));
  p.zinv = (float) (1.0f / vNew.z);
  p.pos3d = vec3(v.position.x, v.position.y, v.position.z);

}

void PixelShader(const Pixel& p, vec3 color, screen* screen, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], Light light, vec4 normal, vec3 reflectance){
  vec3 lightDistance = light.lightPos - p.pos3d;
  float pirdoi = (4 * 3.1415f * length(lightDistance) * length(lightDistance));
  vec3 newNormal(normal.x, normal.y, normal.z);
  vec3 directIllumination = (light.lightPower
    * max(dot(normalize(lightDistance), normalize(newNormal)),0.0f)) / pirdoi;

  vec3 illumination = reflectance * (directIllumination + light.indirectLightPowerPerArea);

  if(p.zinv > depthBuffer[p.y][p.x]){
    depthBuffer[p.y][p.x] = p.zinv;
    PutPixelSDL(screen, p.x, p.y, illumination * color);
  }
}

void Interpolate(Pixel a, Pixel b, vector<Pixel>& result){
  int N = result.size();

  float stepZ= (b.zinv - a.zinv) / float(max(N - 1, 1));
  float stepX = (b.x - a.x) / float(max(N - 1, 1));
  float stepY = (b.y - a.y) / float(max(N - 1, 1));
  vec3 stepPos3d = (b.pos3d * b.zinv - a.pos3d * a.zinv) / float(max(N - 1, 1));

  for (int i = 0; i < result.size(); i++) {
    result[i].x = glm::round(a.x + stepX * i);
    result[i].y = glm::round(a.y + stepY * i);
    result[i].zinv = a.zinv + stepZ * i;
    result[i].pos3d = (a.pos3d * a.zinv + stepPos3d * float(i))/result[i].zinv;
  }
}
void interpolateLine(Pixel a, Pixel b, vector<Pixel> &line) {
  ivec2 ai = ivec2(a.x, a.y);
  ivec2 bi = ivec2(b.x, b.y);
  ivec2 delta = glm::abs(ai - bi);
  int pixels = glm::max(delta.x, delta.y) + 1;

  line.resize(pixels);
  Interpolate(a, b, line);
}

 void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels){
   int yMin = vertexPixels[0].y;
   int yMax = vertexPixels[0].y;
   for (auto vertexPixel : vertexPixels){
     if (vertexPixel.y < yMin)
       yMin = vertexPixel.y;
     else if (vertexPixel.y > yMax)
       yMax = vertexPixel.y;
   }
   int rows = yMax - yMin + 1;

   leftPixels.resize(rows);
   rightPixels.resize(rows);

   for (int i = 0; i < rows; i++){
     leftPixels[i].x = +numeric_limits<int>::max();
     rightPixels[i].x = -numeric_limits<int>::max();
   }

   vector< vector<Pixel> > lines(3);
   for(int i = 0; i < 3; i++){
     interpolateLine(vertexPixels[i], vertexPixels[(i+1)%3], lines[i]);
   }

   for (auto line : lines){
     for (auto pixel : line){
       if (pixel.x < leftPixels[pixel.y - yMin].x)
         leftPixels[pixel.y - yMin] = pixel;
       if (pixel.x > rightPixels[pixel.y - yMin].x)
         rightPixels[pixel.y - yMin] = pixel;
     }
   }
 }

 void DrawPolygonRows(screen *screen, const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels, vec3 color, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], Light light, vec4 normal, vec3 reflectance ){
   for(int i = 0; i < leftPixels.size(); i++){
     vector<Pixel> line(rightPixels[i].x - leftPixels[i].x + 1);
     Interpolate(leftPixels[i], rightPixels[i], line);
     for(auto pixel : line){
       if(pixel.x > 0 && pixel.x < SCREEN_WIDTH && pixel.y > 0 && pixel.y < SCREEN_HEIGHT){
         PixelShader((Pixel) pixel, color, screen, depthBuffer, light, normal, reflectance);
       }
     }
   }
 }

 void DrawPolygon(screen *screen, const vector<Vertex>& vertices, Camera cam, vec3 color, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], Light light, vec4 normal, vec3 reflectance){
   int V = vertices.size();

   vector<Pixel> vertexPixels(V);
   for(int i = 0; i < V; i++){
     VertexShader(vertices[i], vertexPixels[i], cam);
   }
   vector<Pixel> leftPixels;
   vector<Pixel> rightPixels;
   ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
   DrawPolygonRows(screen, leftPixels, rightPixels, color, depthBuffer, light, normal, reflectance);
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

  Light light;
  reset_light(light);
  while (NoQuitMessageSDL())
    {
      Update(cam, light);
      Draw(screen, triangles, cam, light);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen, vector<Triangle>& triangles, Camera cam, Light light)
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
    vec4 currentNormal = triangles[i].normal;
    vec3 currentReflectance(1,1,1);

    vector<Vertex> vertices(3);
    vertices[0].position = triangles[i].v0;
    vertices[1].position = triangles[i].v1;
    vertices[2].position = triangles[i].v2;


    DrawPolygon(screen, vertices, cam, triangles[i].color, depthBuffer, light, currentNormal, currentReflectance);
  }
}

/*Place updates of parameters here*/
void Update(Camera &cam, Light &light)
{
  // static int t = SDL_GetTicks();
  // /* Compute frame time */
  // int t2 = SDL_GetTicks();
  // float dt = float(t2-t);
  // t = t2;
  // /*Good idea to remove this*/
  // //std::cout << "Render time: " << dt << " ms." << std::endl;
  // /* Update variables*/

  const uint8_t *keyState = SDL_GetKeyboardState(NULL);
  int lastRecordedX = 0;
  int lastRecordedY = 0;
  SDL_GetRelativeMouseState(&lastRecordedX, &lastRecordedY);
  if( keyState[SDL_SCANCODE_W] ){
    cam.cameraPos += (cam.cameraRotationX * cam.cameraRotationY * cam.cameraRotationZ) * vec4(0, 0, 0.02f, 0);
  }
  if( keyState[SDL_SCANCODE_S] ){
    cam.cameraPos -= (cam.cameraRotationX * cam.cameraRotationY * cam.cameraRotationZ) * vec4(0, 0, 0.02f, 0);
  }
  if( keyState[SDL_SCANCODE_A] ){
    cam.cameraPos -= (cam.cameraRotationX * cam.cameraRotationY * cam.cameraRotationZ) * vec4(0.02f, 0, 0, 0);
  }
  if( keyState[SDL_SCANCODE_D] ){
    cam.cameraPos += (cam.cameraRotationX * cam.cameraRotationY * cam.cameraRotationZ) * vec4(0.02f, 0, 0, 0);
  }

  if( keyState[SDL_SCANCODE_UP] ){
    light.lightPos += vec3(0,0,0.02f);
  }
  if( keyState[SDL_SCANCODE_DOWN] ){
    light.lightPos -= vec3(0,0,0.02f);  
    }
  if( keyState[SDL_SCANCODE_LEFT] ){
    light.lightPos -= vec3(0.02f,0,0);  
    }
  if( keyState[SDL_SCANCODE_RIGHT] ){
    light.lightPos += vec3(0.02f,0,0);  
    }
    if( keyState[SDL_SCANCODE_RIGHTBRACKET] ){
    light.lightPos -= vec3(0,0.02f,0);  
    }
    if( keyState[SDL_SCANCODE_BACKSLASH] ){
    light.lightPos += vec3(0,0.02f,0);  
    }




  if( lastRecordedX < 0){
    cam.yaw -= 0.01;
    rotateY(cam, cam.yaw);
  }
  if( lastRecordedX > 0){
    cam.yaw += 0.01;
    rotateY(cam, cam.yaw);
  }
  if( lastRecordedY < 0){
    cam.pitch += 0.01;
    rotateX(cam, cam.pitch);
  }
  if( lastRecordedY > 0){
    cam.pitch -= 0.01;
    rotateX(cam, cam.pitch);
  }

  if( keyState[SDL_SCANCODE_SPACE]){
    reset_camera(cam);
    reset_light(light);
  }
}
