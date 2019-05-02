#include <iostream>
#include <glm/glm.hpp>
#include "utils.cpp"
#include <stdint.h>
// #include <omp.h>
#include "OBJ_Loader.h"
#include "FXAA.cpp"
#include <glm/gtc/type_ptr.hpp>

// OMP_NUM_THREADS

#define FOCAL_LENGTH SCREEN_HEIGHT

void transformToClippingSpace(vector<vec4>& vertices, Camera cam);
int clipTriangleAxis(vector<Vertex>& vertices, vector<Vertex>& resultingList, vector<VertexProperties> &properties, vector<VertexProperties> &resultingProperties, int axis);
void BarycentricDrawPixels(vector<Pixel>& vertexPixels, vec3 color, screen* screen, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH], Light light, Camera lightCamera, vec4 normal, vec3 reflectance, int hasTexture, SDL_Surface* texture_surface);
void perspectiveDivide(vector<Vertex>& vertices);

const float f = 10;
// const float n = SCREEN_HEIGHT;
const float n = 2;
const float t = 1;
const float r = 1;
const float l = -1;
const float b = -1;

// float CLIPPING_ARR[16] = {
//         2 , 0               , 0                      , 0  ,
//         0               , 2 , 0                      , 0  ,
//         0 , 0 , - (f + n)/(f - n)      , -1 ,
//         0               , 0               , -(2 * f * n) / (f - n) , 0
// };

float CLIPPING_ARR[16] = {
        2 * n / (r - l) , 0               , 0                      , 0  ,
        0               , 2 * n / (t - b) , 0                      , 0  ,
        (r + l)/(r - l) , (t + b)/(t - b) , - (f + n)/(f - n)      , -1 ,
        0               , 0               , -(2 * f * n) / (f - n) , 0
};

float _VIEWPORT_TRANSFORM_ARRAY[16] = {
0.5 , 0   , 0   , 0 ,
0   , 0.5 , 0   , 0 ,
0   , 0   , 0.5 , 0 ,
0.5 , 0.5 , 0.5 , 1
};

// float CLIPPING_ARR[16] = {
//         SCREEN_WIDTH/2 , 0               , 0                      , SCREEN_WIDTH/2  ,
//         0               , -SCREEN_HEIGHT/2 , 0                      , SCREEN_HEIGHT/2  ,
//         0 , 0 , 1      , 0 ,
//         0               , 0               , 0 , 1
// };
mat4 CLIP_SPACE_TRANSFORM;
mat4 VIEWPORT_TRANSFORM;


vec3 currentPixels[SCREEN_HEIGHT][SCREEN_WIDTH];
int currentMinimumX = 0, currentMinimumY = 0;
int currentMaximumX = SCREEN_HEIGHT, currentMaximumY = SCREEN_WIDTH;
int doAntiAliasing = 0;
int doShadows = 0;
int doClipping = 0;

vector<vec3> prevPos3ds;
vec3 zero(0, 0, 0);
// w = z / f

Camera getLightCamera(Light light){

   Camera lightCamera;
   lightCamera.cameraRotationY = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
   lightCamera.cameraRotationX = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
   lightCamera.cameraRotationZ = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
   lightCamera.yaw = 0.0;
   lightCamera.pitch = 0.0;
   lightCamera.focalLength = SCREEN_HEIGHT/2;
   lightCamera.cameraPos = vec4(light.lightPos.x, light.lightPos.y, light.lightPos.z, 1);
  //  lightCamera.pitch -= 2.0f;
  //  rotateX(lightCamera, lightCamera.pitch);
   return lightCamera;
}

vec3 prevPos3d(0,0,0);

// inline void VertexShaderClipping(const Vertex& v, Pixel& p, Camera cam){
//   // vec4 vNew = (v.position - cam.cameraPos) * (cam.cameraRotationX * cam.cameraRotationY);
//   vec4 vNew = v.position;
//   // p.pos3d = vec3(v.position.x, v.position.y, v.position.z);
//   p.pos3d = vec3(prevPos3d.x, prevPos3d.y, prevPos3d.z);

//   // if(vNew.z < 0.001)
//   //   return;

//   // vNew.w = v.position.w * (cam.cameraRotationX * cam.cameraRotationY);
//   // cout << cam.focalLength << " " << vNew.z << " " << vNew.w << " " << v.position.w << " A: " << vNew.x / v.position.w + (SCREEN_WIDTH / 2) << " B: " <<  (cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2) << endl;
//   // p.x = (int) ((cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2));
//   p.x = (int) ((vNew.x / vNew.w) + (SCREEN_WIDTH / 2));
//   // p.y = (int) ((cam.focalLength * vNew.y / vNew.z)  + (SCREEN_HEIGHT / 2));
//   p.y = (int) ((vNew.y / vNew.w) + (SCREEN_HEIGHT) / 2);

//   // if(doClipping){
//   //   p.x = (int) ((cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2));
//   //   p.y = (int) ((cam.focalLength * vNew.y / vNew.z)  + (SCREEN_HEIGHT / 2));
//   // }

//   // IMPARTE LA vNew.w
//   p.zinv = (float) (1.0f / vNew.z);
//   // p.zinv = (float) 1.0f / (cam.focalLength * vNew.w);
  
//   p.textureX = v.texturePosition.x;
//   p.textureY = v.texturePosition.y;
// }

inline void VertexShader(const Vertex& v, Pixel& p, Camera cam){

  vec4 vNew = (v.position - cam.cameraPos) * (cam.cameraRotationX * cam.cameraRotationY);

  p.pos3d = vec3(v.position.x, v.position.y, v.position.z);
  prevPos3d = p.pos3d;

  // if(vNew.z < 0.001)
  //   return;

  // vNew.w = v.position.w * (cam.cameraRotationX * cam.cameraRotationY);
  // cout << cam.focalLength << " " << vNew.z << " " << vNew.w << " " << v.position.w << " A: " << vNew.x / v.position.w + (SCREEN_WIDTH / 2) << " B: " <<  (cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2) << endl;
  p.x = (int) ((cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2));
  // p.x = (int) ((vNew.x / vNew.w) + (SCREEN_WIDTH / 2));
  p.y = (int) ((cam.focalLength * vNew.y / vNew.z)  + (SCREEN_HEIGHT / 2));
  // p.y = (int) ((vNew.y / vNew.w) + (SCREEN_HEIGHT) / 2);

  // IMPARTE LA vNew.w
  p.zinv = (float) (1.0f / vNew.z);
  // p.zinv = (float) 1.0f / (cam.focalLength * vNew.w);
  
  p.textureX = v.texturePosition.x;
  p.textureY = v.texturePosition.y;
}

void VertexShaderClipping(const Vertex& v, VertexProperties &props, Pixel &p){
  // p.pos3d = vec3(props.relativePosition.x, props.relativePosition.y, props.relativePosition.z);
  p.pos3d = props.initialPosition;
  vec4 pixelVertex = VIEWPORT_TRANSFORM * v.position;
  p.x = (v.position.x / v.position.w) + (SCREEN_WIDTH / 2);
  p.y = (v.position.y / v.position.w) + (SCREEN_HEIGHT / 2);
  // p.x = int(SCREEN_WIDTH * pixelVertex.x);
  // p.y = int(SCREEN_HEIGHT * pixelVertex.y);

  // p.zinv = (float) (1.0f / v.position.z);
  p.zinv = props.zinv;

  p.textureX = props.texturePos.x;
  p.textureY = props.texturePos.y;
}

void RememberPropertiesClipping(const Vertex &v, VertexProperties &properties, Camera cam){
  properties.initialPosition = vec3(v.position.x, v.position.y, v.position.z);
  properties.texturePos = v.texturePosition;
  vec4 vNew = (v.position - cam.cameraPos) * (cam.cameraRotationX * cam.cameraRotationY);
  properties.relativePosition = vNew;
  properties.zinv = 1 / vNew.z;
}

void PixelShader(const Pixel& p, vec3 color, screen* screen, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH], Light light, vec4 normal, vec3 reflectance){
  vec3 lightDistance = light.lightPos - p.pos3d;
  float pirdoi = (4 * 3.1415f * length(lightDistance) * length(lightDistance));
  vec3 newNormal(normal.x, normal.y, normal.z);
  vec3 directIllumination = (light.lightPower
    * max(dot(normalize(lightDistance), normalize(newNormal)),0.0f)) / pirdoi;

  vec3 illumination = reflectance * (directIllumination + light.indirectLightPowerPerArea);

  // cout << "INSIDE PIXEL SHADER " << endl;
  if(p.zinv > depthBuffer[p.y][p.x]){
    depthBuffer[p.y][p.x] = p.zinv;
    

    if(p.x >= 0 && p.x < SCREEN_HEIGHT && p.y >= 0 && p.y < SCREEN_WIDTH){


      if(!doAntiAliasing){
        PutPixelSDL(screen, p.x, p.y, illumination * color);
        // cout << "PUT PIXEL SDL " << p.x << " " << p.y << " " << endl;
      }
      else{
        currentPixels[p.x][p.y] = (illumination * color);
        currentMaximumX = std::max(currentMaximumX, p.x + 1);
        currentMaximumY = std::max(currentMaximumY, p.y + 1);
        currentMinimumX = std::min(currentMinimumX, p.x);
        currentMinimumY = std::min(currentMinimumY, p.y);
      }
    }
  }
}

void PixelShader(const Pixel& p, const Pixel& cameraPixel, vec3 color, screen* screen, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH], Light light, vec4 normal, vec3 reflectance){
  vec3 lightDistance = light.lightPos - p.pos3d;
  float pirdoi = (4 * 3.1415f * length(lightDistance) * length(lightDistance));
  vec3 newNormal(normal.x, normal.y, normal.z);
  vec3 directIllumination = (light.lightPower
    * max(dot(normalize(lightDistance), normalize(newNormal)),0.0f)) / pirdoi;

  vec3 illumination = reflectance * (directIllumination + light.indirectLightPowerPerArea);

  // cout << "INSIDE PIXEL SHADER " << endl;
  if(p.zinv > depthBuffer[p.y][p.x]){
    depthBuffer[p.y][p.x] = p.zinv;
    

    if(p.x >= 0 && p.x < SCREEN_HEIGHT && p.y >= 0 && p.y < SCREEN_WIDTH){

      vec3 toPutPixel = illumination*color;
        // cout << p.zinv << " " << shadow
      if(doShadows){
        if((cameraPixel.zinv < shadowMap[cameraPixel.y][cameraPixel.x] && abs(shadowMap[cameraPixel.y][cameraPixel.x] - cameraPixel.zinv) > 0.01f)){
          toPutPixel.x /= 2;
          toPutPixel.y /= 2;
          toPutPixel.z /= 2;
        }
      }

      if(!doAntiAliasing){
        
        PutPixelSDL(screen, p.x, p.y, toPutPixel);
        // cout << "PUT PIXEL SDL " << p.x << " " << p.y << " " << endl;
      }
      else{
        currentPixels[p.x][p.y] = toPutPixel;
        currentMaximumX = std::max(currentMaximumX, p.x + 1);
        currentMaximumY = std::max(currentMaximumY, p.y + 1);
        currentMinimumX = std::min(currentMinimumX, p.x);
        currentMinimumY = std::min(currentMinimumY, p.y);
      }
    }
  }
}

void Interpolate(Pixel a, Pixel b, vector<Pixel>& result){
  int N = result.size();

  float stepZ= (b.zinv - a.zinv) / float(max(N - 1, 1));
  float stepX = (b.x - a.x) / float(max(N - 1, 1));
  float stepY = (b.y - a.y) / float(max(N - 1, 1));
  vec3 stepPos3d = (b.pos3d * b.zinv - a.pos3d * a.zinv) / float(max(N - 1, 1));

  for (size_t i = 0; i < result.size(); i++) {
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

int isInsideFrustrum(Vertex v){
      vec4 position = v.position;
			return 
				abs(position.x) <= abs(position.w) &&
				abs(position.y) <= abs(position.w) &&
				abs(position.z) <= abs(position.w);
}

 void DrawPolygon(screen *screen, vector<Vertex>& vertices, Camera cam, vec3 color, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH], Light light, Camera lightCamera, vec4 normal, vec3 reflectance, bool hasTexture, SDL_Surface* texture_surface){
   int V = vertices.size();
   vector<Pixel> vertexPixels(V);
   
  //  if(!doClipping){
    
  //  }else if(afterClipping){
  //    for(int i = 0; i < V; i++){
  //     VertexShaderClipping(vertices[i], vertexPixels[i], cam);
  //   }
  //  }
  
  if(doClipping){
      vector<vec4> positions(3);
      vector<VertexProperties> properties(3);
      
      // VertexShaderClipping
      for(int i = 0; i < V; i++){
        RememberPropertiesClipping(vertices[i], properties[i], cam);
        positions[i] = ((vertices[i].position - cam.cameraPos) * (cam.cameraRotationX * cam.cameraRotationY));
      }

      // positions.push_back(vertices[0].position);
      // positions.push_back(vertices[1].position);
      // positions.push_back(vertices[2].position);
      // cout << "POS BEFORE " << positions[0].x << " " << positions[0].y << " " << positions[0].z << " " << positions[0].w << endl;
      transformToClippingSpace(positions, cam);
      // cout << "POS AFTER " << positions[0].x << " " << positions[0].y << " " << positions[0].z << " " << positions[0].w << endl;

      // if(isInsideFrustrum(vertices[0]) && isInsideFrustrum(vertices[1]) && isInsideFrustrum(vertices[2])){
      //   DrawPolygon(screen, vertices, cam, triangles[i].color,
      //           depthBuffer, light, currentNormal, currentReflectance, triangles[i].hasTexture, NULL);

      //   cout << "HAS DRAWNNNNNNNN" << endl;
      //   continue;
      // }
      
      vertices[0].position = positions[0];
      vertices[1].position = positions[1];
      vertices[2].position = positions[2];

      vector<Vertex> resultingVertices;
      vector<VertexProperties> resultingProperties;
      if(clipTriangleAxis(vertices, resultingVertices, properties, resultingProperties, 0) &&
      clipTriangleAxis(vertices, resultingVertices, properties, resultingProperties, 1)){

        // cout << vertices.size() << endl;

        // perspectiveDivide(vertices);

        //  for(size_t v = 0; v < vertices.size(); v++){
        //    vertices[v].position.x = vertices[v].position.x / vertices[v].position.w;
        //    vertices[v].position.y = vertices[v].position.y / vertices[v].position.w;
        //    vertices[v].position.z = vertices[v].position.z / vertices[v].position.w;
        //    vertices[v].position.w = 1;
        //  }

         vector<VertexProperties> currentTriangleProperties(3);
         vector<Pixel> currentPixels(3);

         vector<Pixel> pixels(vertices.size());

         for(size_t v = 0; v < vertices.size(); v++){
            // if(vertices[v].position.w == 0)
            //   vertices[v].position.w = 1;

            VertexShaderClipping(vertices[v], properties[v], pixels[v]);
         }

        //  cout << " ---------------------------------------- " << endl;
        //   for (int i = 0; i < vertices.size(); i++){
        //     cout << "VERTEX CLIP " << i << " IS " << vertices[i].position.x << " " << vertices[i].position.y << " " << vertices[i].position.z << endl;
        //   }

        //   cout << " ---------------------------------------- " << endl;
          // for (int i = 0; i < pixels.size(); i++){
          //   PutPixelSDL(screen, pixels[i].x, pixels[i].y, vec3(0.75f, 0.15f, 0.15f));
          //   PutPixelSDL(screen, pixels[i].x-1, pixels[i].y, vec3(0.75f, 0.15f, 0.15f));
          //   PutPixelSDL(screen, pixels[i].x+1, pixels[i].y, vec3(0.75f, 0.15f, 0.15f));
          //   PutPixelSDL(screen, pixels[i].x, pixels[i].y-1, vec3(0.75f, 0.15f, 0.15f));
          //   PutPixelSDL(screen, pixels[i].x-1, pixels[i].y-1, vec3(0.75f, 0.15f, 0.15f));
          //   PutPixelSDL(screen, pixels[i].x+1, pixels[i].y-1, vec3(0.75f, 0.15f, 0.15f));
          //   PutPixelSDL(screen, pixels[i].x, pixels[i].y+1, vec3(0.75f, 0.15f, 0.15f));
          //   PutPixelSDL(screen, pixels[i].x-1, pixels[i].y+1, vec3(0.75f, 0.15f, 0.15f));
          //   PutPixelSDL(screen, pixels[i].x+1, pixels[i].y+1, vec3(0.75f, 0.15f, 0.15f));
          // }
        //  cout << " ---------------------------------------- " << endl;
        //   for (int i = 0; i < pixels.size(); i++){
        //     cout << "PIXEL " << i << " IS " << pixels[i].x << " " << pixels[i].y << endl;
        //   }

        //   cout << " ---------------------------------------- " << endl;
         
          for(size_t v = 1; v < vertices.size()-1; v++){

              vector<Vertex> currentTriangleVertices(3);

              currentTriangleVertices[0] = vertices[0];
              currentTriangleVertices[1] = vertices[v];
              currentTriangleVertices[2] = vertices[v+1];


              currentTriangleProperties[0] = properties[0];
              currentTriangleProperties[1] = properties[v];
              currentTriangleProperties[2] = properties[v+1];

              currentPixels[0] = pixels[0];
              currentPixels[1] = pixels[v];
              currentPixels[2] = pixels[v+1];
              
              // for (int i = 0; i < currentPixels.size(); i++){
              //   cout << "CURRENT PIXEL " << i << " IS " << currentPixels[i].x << " " << currentPixels[i].y << endl;
              // }

              // cout << "AFTER ONE: " << currentTriangleVertices[0].position.x << " " << currentTriangleVertices[0].position.y << " " << currentTriangleVertices[0].position.z << " " << currentTriangleVertices[0].position.w << " " << endl;
              // cout << "AFTER TWO: " << currentTriangleVertices[1].position.x << " " << currentTriangleVertices[1].position.y << " " << currentTriangleVertices[1].position.z << " " << currentTriangleVertices[v].position.w << " " <<endl;
              // cout << "AFTER THREE: " << currentTriangleVertices[2].position.x << " " << currentTriangleVertices[2].position.y << " " << currentTriangleVertices[2].position.z << " " << currentTriangleVertices[v+1].position.w << " " <<endl;

              // cout << "SHOULD BE DRAWING?" << endl;

              BarycentricDrawPixels(currentPixels, color, screen, depthBuffer, shadowMap, light, lightCamera, normal, reflectance, hasTexture, texture_surface);
            }
          
      }
      
      
      // for(int i = 0; i < V; i++){
      //   VertexShaderClipping(vertices[i], properties[i], vertexPixels[i]);
      // }
      // cout << "Positions: "<< positions[0].x << " " << positions[0].y << " " << positions[0].z << " " << positions[0].w << endl
      //   << positions[1].x << " " << positions[1].y << " " << positions[1].z << " " << positions[1].w << endl
      //   << positions[2].x << " " << positions[2].y << " " << positions[2].z << " " << positions[2].w << endl;
      // cout << "IN DRAW" << endl;
      // BarycentricDrawPixels(vertexPixels, color, screen, depthBuffer, shadowMap, light, lightCamera, normal, reflectance, hasTexture, texture_surface);
      
      return;
  }else{
    for(int i = 0; i < V; i++){
      VertexShader(vertices[i], vertexPixels[i], cam);
    }
    BarycentricDrawPixels(vertexPixels, color, screen, depthBuffer, shadowMap, light, lightCamera, normal, reflectance, hasTexture, texture_surface);
  }
 }


 void BarycentricDrawPixels(vector<Pixel>& vertexPixels, vec3 color, screen* screen, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH], Light light, Camera lightCamera, vec4 normal, vec3 reflectance, int hasTexture, SDL_Surface* texture_surface){

   int maxX = -numeric_limits<int>::max(); 
   int minX = numeric_limits<int>::max();;
   int maxY = -numeric_limits<int>::max();
   int minY = numeric_limits<int>::max();;
   
   for(size_t i = 0; i < vertexPixels.size(); i++){
     maxX = std::max(maxX, vertexPixels[i].x);
     maxY = std::max(maxY, vertexPixels[i].y);
     minX = std::min(minX, vertexPixels[i].x);
     minY = std::min(minY, vertexPixels[i].y);
   }

  //  cout << minX << " " << maxX << "    " << minY << " " << maxY << endl;
  // cout << "ENTERS DRAW POLYGON " << endl;
   for(int y = minY; y < maxY; y++){
     if(y >= SCREEN_HEIGHT || y < 0)
      continue;

    //  #pragma omp parallel for
     for(int x = minX; x < maxX; x++){
       if(x < 0 || x >= SCREEN_WIDTH)
        continue;
       glm::vec2 v0(vertexPixels[1].x - vertexPixels[0].x, vertexPixels[1].y - vertexPixels[0].y);
       glm::vec2 v1(vertexPixels[2].x - vertexPixels[0].x, vertexPixels[2].y - vertexPixels[0].y);
       glm::vec2 v2(x - vertexPixels[0].x, y - vertexPixels[0].y);
       
       Pixel tPixel;
       
       tPixel.x = x;
       tPixel.y = y;

       float dot00 = glm::dot(v0, v0);
       float dot01 = glm::dot(v0, v1);
       float dot11 = glm::dot(v1, v1);
       float dot20 = glm::dot(v2, v0);
       float dot21 = glm::dot(v2, v1);

       float denom = dot00 * dot11 - dot01 * dot01;
       float a = (dot11 * dot20 - dot01 * dot21) / denom; // v1
       float b = (dot00 * dot21 - dot01 * dot20) / denom; // v2
       float c = 1.0f - a - b; // v0
      //  cout << "JUST BEFORE IF a: " << a << " b: " << b << " c: " << c << endl;
       if (0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1){
        // cout << "IF IS RESPECTED" << endl;
        tPixel.zinv = vertexPixels[1].zinv*a + vertexPixels[2].zinv*b + vertexPixels[0].zinv*c;
        tPixel.pos3d = (vertexPixels[1].pos3d * vertexPixels[1].zinv*a + vertexPixels[2].pos3d * vertexPixels[2].zinv*b + vertexPixels[0].pos3d * vertexPixels[0].zinv*c) / tPixel.zinv;
        // tPixel.zinv = vertexPixels[0].zinv*a + vertexPixels[1].zinv*b + vertexPixels[2].zinv*c;
        // tPixel.pos3d = (vertexPixels[0].pos3d * vertexPixels[0].zinv*a + vertexPixels[1].pos3d * vertexPixels[1].zinv*b + vertexPixels[2].pos3d * vertexPixels[2].zinv*c) / tPixel.zinv;
        if(hasTexture && texture_surface != NULL){
          color = getTextureAt(texture_surface, vertexPixels[1].textureX*a+vertexPixels[2].textureX*b+vertexPixels[0].textureX*c,
            vertexPixels[1].textureY*a+vertexPixels[2].textureY*b+vertexPixels[0].textureY*c);
          // color = (
          //           (getTextureAt(texture_surface, vertexPixels[1].textureX, vertexPixels[1].textureY) * a) +
          //           (getTextureAt(texture_surface, vertexPixels[2].textureX, vertexPixels[2].textureY) * b) +
          //           (getTextureAt(texture_surface, vertexPixels[0].textureX, vertexPixels[0].textureY) * c)
          //         );
        }

        if(doShadows){
          Pixel cameraPixel;
          cameraPixel.pos3d = tPixel.pos3d;
          vec4 vNew = (vec4(cameraPixel.pos3d.x,cameraPixel.pos3d.y,cameraPixel.pos3d.z, 1) - lightCamera.cameraPos) * lightCamera.cameraRotationX;
          if(vNew.z < 0.001){
            PixelShader(tPixel, color, screen, depthBuffer, shadowMap, light, normal, reflectance);
          }else{
            cameraPixel.x = (int) ((lightCamera.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2));
            cameraPixel.y = (int) ((lightCamera.focalLength * vNew.y / vNew.z)  + (SCREEN_HEIGHT / 2));
            cameraPixel.zinv = 1.0f / vNew.z;
            if(cameraPixel.y >= 0 && cameraPixel.y < SCREEN_HEIGHT && cameraPixel.x >= 0 && cameraPixel.x < SCREEN_WIDTH &&
              abs(vNew.z - lightCamera.cameraPos.z) >= 0.1f){
              PixelShader(tPixel, cameraPixel, color, screen, depthBuffer, shadowMap, light, normal, reflectance);
            }else{
              PixelShader(tPixel, color, screen, depthBuffer, shadowMap, light, normal, reflectance);
            }
            
          }
        }else{
          PixelShader(tPixel, color, screen, depthBuffer, shadowMap, light, normal, reflectance);
        }
       }
     }
   }
 }

inline void VertexShaderShadow(const Vertex& v, Pixel& p, Camera cam){

    vec4 vNew = (v.position - cam.cameraPos) * cam.cameraRotationX;

    if(vNew.z < 0.00001)
      return;
    // vNew.w = v.position.w * (cam.cameraRotationX * cam.cameraRotationY);
    // cout << cam.focalLength << " " << vNew.z << " " << vNew.w << " " << v.position.w << " A: " << vNew.x / v.position.w + (SCREEN_WIDTH / 2) << " B: " <<  (cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2) << endl;
    p.x = (int) ((cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2));
    // p.x = (int) ((vNew.x / vNew.w) + (SCREEN_WIDTH / 2));
    p.y = (int) ((cam.focalLength * vNew.y / vNew.z)  + (SCREEN_HEIGHT / 2));
    // p.y = (int) ((vNew.y / vNew.w) + (SCREEN_HEIGHT) / 2);

    // IMPARTE LA vNew.w
    p.zinv = (float) (1.0f / vNew.z);
    // p.zinv = (float) 1.0f / (cam.focalLength * vNew.w);
    p.pos3d = vec3(v.position.x, v.position.y, v.position.z);
    p.textureX = v.texturePosition.x;
    p.textureY = v.texturePosition.y;
  }




 void DrawShadowMapForTriangle(screen *screen, const vector<Vertex> &vertices, Light light, Camera lightCamera, float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH]){
   int V = vertices.size();
   vector<Pixel> vertexPixels(V);
   

   for(int i = 0; i < V; i++){
      VertexShaderShadow(vertices[i], vertexPixels[i], lightCamera);
   }

   int maxX = -numeric_limits<int>::max(); 
   int minX = numeric_limits<int>::max();;
   int maxY = -numeric_limits<int>::max();
   int minY = numeric_limits<int>::max();;
   
   for(size_t i = 0; i < vertexPixels.size(); i++){
     maxX = std::max(maxX, vertexPixels[i].x);
     maxY = std::max(maxY, vertexPixels[i].y);
     minX = std::min(minX, vertexPixels[i].x);
     minY = std::min(minY, vertexPixels[i].y);
   }

  //  cout << minX << " " << maxX << "    " << minY << " " << maxY << endl;
  // cout << "ENTERS DRAW POLYGON " << endl;
   for(int y = minY; y < maxY; y++){
     if(y >= SCREEN_HEIGHT || y < 0)
      continue;

    //  #pragma omp parallel for
     for(int x = minX; x < maxX; x++){
       if(x < 0 || x >= SCREEN_WIDTH)
        continue;
       glm::vec2 v0(vertexPixels[1].x - vertexPixels[0].x, vertexPixels[1].y - vertexPixels[0].y);
       glm::vec2 v1(vertexPixels[2].x - vertexPixels[0].x, vertexPixels[2].y - vertexPixels[0].y);
       glm::vec2 v2(x - vertexPixels[0].x, y - vertexPixels[0].y);
       
       Pixel tPixel;
       tPixel.x = x;
       tPixel.y = y;

       float dot00 = glm::dot(v0, v0);
       float dot01 = glm::dot(v0, v1);
       float dot11 = glm::dot(v1, v1);
       float dot20 = glm::dot(v2, v0);
       float dot21 = glm::dot(v2, v1);

       float denom = dot00 * dot11 - dot01 * dot01;
       float a = (dot11 * dot20 - dot01 * dot21) / denom; // v1
       float b = (dot00 * dot21 - dot01 * dot20) / denom; // v2
       float c = 1.0f - a - b; // v0
      //  cout << "JUST BEFORE IF a: " << a << " b: " << b << " c: " << c << endl;
       if (0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1){
        // cout << "IF IS RESPECTED" << endl;
        tPixel.zinv = vertexPixels[1].zinv*a + vertexPixels[2].zinv*b + vertexPixels[0].zinv*c;
        // tPixel.pos3d = (vertexPixels[1].pos3d * vertexPixels[1].zinv*a + vertexPixels[2].pos3d * vertexPixels[2].zinv*b + vertexPixels[0].pos3d * vertexPixels[0].zinv*c) / tPixel.zinv;

        if((tPixel.zinv > shadowMap[tPixel.y][tPixel.x]) || shadowMap[tPixel.y][tPixel.x] == 0.0f){
          shadowMap[tPixel.y][tPixel.x] = tPixel.zinv;
        }
      }
    }
  }
 }





int main( int argc, char* argv[] )
{
  // memcpy( glm::value_ptr( CLIP_SPACE_TRANSFORM ), CLIPPING_ARR, sizeof( CLIPPING_ARR ) );
  CLIP_SPACE_TRANSFORM = glm::make_mat4(CLIPPING_ARR);
  VIEWPORT_TRANSFORM = glm::make_mat4(_VIEWPORT_TRANSFORM_ARRAY);
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  SDL_SetRelativeMouseMode(SDL_TRUE);

  vector<Triangle> triangles;
  srand(time(NULL));

  LoadTestModel(triangles);
  int sceneCount = triangles.size();
  for(int i = 0; i < sceneCount; i++){
    triangles[i].hasTexture = false;
  }

  vector<RenderedObject> objects;
  // RenderedObject temple = LoadObject("Objects/Temple/temple.obj");
  // normaliseTriangles(temple.triangles,
  //                    1.4, 
  //                    0.6, -0.4, 0.1,
  //                    3.1, 0, 0,
  //                    1, -1, -1);
  // temple.texture_surface = SDL_LoadBMP("Objects/Temple/temple.bmp");
  // objects.push_back(temple);

  RenderedObject dog = LoadObject("Objects/DogMaya/dog.obj");
  normaliseTriangles(dog.triangles,
                     1.4, 
                     0.6, -0.4, 0.1,
                     1.8f, 0, 0,
                     1, -1, -1);
  dog.texture_surface = SDL_LoadBMP("Objects/DogMaya/Dog_diffuse.bmp");
  objects.push_back(dog);
                       
  Camera cam;
  reset_camera(cam);

  Light light;
  reset_light(light, doShadows);
  while (NoQuitMessageSDL())
    {
      Update(cam, light);
      Draw(screen, triangles, objects, cam, light);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

Vertex interpolate(Vertex first, Vertex last, float t) {
  Vertex i;
  // (last - first) * t + first
  // if(t >= -1.01 && t <= 1.01){
  i.position.x = first.position.x * (1 - t) + last.position.x * t;
  i.position.y = first.position.y * (1 - t) + last.position.y * t;
  i.position.z = first.position.z * (1 - t) + last.position.z * t;
  i.position.w = i.position.z / FOCAL_LENGTH;
  // first.position.w * (1 - t) + last.position.w * t;

  i.normal = first.normal;
  i.reflectance = first.reflectance;

  i.texturePosition.x = first.texturePosition.x * (1 - t) + last.texturePosition.x * t;
  i.texturePosition.y = first.texturePosition.y * (1 - t) + last.texturePosition.y * t;
  i.texture = first.texture;
  // }
  return i;

}

VertexProperties interpolateProperties(VertexProperties first, VertexProperties last, float amount){
  VertexProperties result;
  // cout << "INTERPOLATES PROPERTIES, AMOUNT IS " << amount << endl; 
  result.zinv = first.zinv * (1-amount) + last.zinv * amount;
  result.relativePosition = first.relativePosition * (1-amount) + last.relativePosition * amount;
  result.initialPosition = first.initialPosition * (1 - amount) + last.initialPosition * amount;
  result.texturePos.x = first.texturePos.x * (1 - amount) + last.texturePos.x * amount;
  result.texturePos.y = first.texturePos.y * (1 - amount) + last.texturePos.y * amount;
  return result;
}

float getPositionByAxis(Vertex v, int axis){
  if(axis == 0) return v.position.x;
  if(axis == 1) return v.position.y;
  if(axis == 2) return v.position.z;
  return v.position.w;
}

void clipHelper(vector<Vertex> &vertices, vector<Vertex> &resultingVertices, 
  vector<VertexProperties> &properties, vector<VertexProperties> &resultingProperties, int axis, float factor){
  // cout << " ---------------------------------------- " << endl;
  // for (int i = 0; i < vertices.size(); i++){
  //   cout << "INITIAL VERTEX " << i << " IS " << vertices[i].position.x << " " << vertices[i].position.y << " " << vertices[i].position.z << endl;
  // }

  // cout << " ---------------------------------------- " << endl;
  Vertex previousVertex = vertices[vertices.size() - 1];
  VertexProperties previousProperties = properties[properties.size() - 1];
  float previousComponent = (getPositionByAxis(previousVertex, axis)) * factor;
  unsigned int previousInside = (previousComponent <= previousVertex.position.w);
  for(size_t i = 0; i < vertices.size(); i++){
    Vertex currentVertex = vertices[i];
    VertexProperties currentProperties = properties[i];
    float currentComponent = (getPositionByAxis(currentVertex, axis)) * factor;
    int currentInside = currentComponent <= currentVertex.position.w;
    // cout << "Current component " << currentComponent << ", current w " << currentVertex.position.w << endl;
    
    if(currentInside ^ previousInside){
      float interpolateAmount = (previousVertex.position.w - previousComponent) /
        ((previousVertex.position.w - previousComponent) -
         (currentVertex.position.w - currentComponent));
      
      Vertex toPushBack;
      toPushBack.position = previousVertex.position + interpolateAmount * (currentVertex.position - previousVertex.position);
      resultingVertices.push_back(toPushBack);
      // cout << "CURRENT VERTEX POSITION IS " << currentVertex.position.x << " " << currentVertex.position.y << " " << currentVertex.position.z << " " << currentVertex.position.w << endl;
      // cout << "PREVIOUS VERTEX POSITION IS " << previousVertex.position.x << " " << previousVertex.position.y << " " << previousVertex.position.z << " " <<previousVertex.position.w << endl;
      // cout << "TO PUSH VERTEX POSITION IS " << toPushBack.position.x << " " << toPushBack.position.y << " " << toPushBack.position.z <<" " << toPushBack.position.w << endl;
      resultingProperties.push_back(interpolateProperties(previousProperties, currentProperties, interpolateAmount));
      // resultingVertices.push_back(interpolate(previousVertex, currentVertex, interpolateAmount));
    }

    if(currentInside){
      resultingVertices.push_back(currentVertex);
      resultingProperties.push_back(currentProperties);
    }
    previousVertex = currentVertex;
    previousProperties = currentProperties;
    previousComponent = currentComponent;
    previousInside = currentInside;
    // float currente
  }
  // cout << " ---------------------------------------- " << endl;
  // for (int i = 0; i < resultingVertices.size(); i++){
  //   cout << "RESULTING VERTEX " << i << " IS " << resultingVertices[i].position.x << " " << resultingVertices[i].position.y << " " << resultingVertices[i].position.z << " " << resultingVertices[i].position.w << endl;
  // }

  // cout << " ---------------------------------------- " << endl;
};


// 0.00313f

int clipTriangleAxis(vector<Vertex>& vertices, vector<Vertex>& resultingList, vector<VertexProperties> &properties, vector<VertexProperties> &resultingProperties, int axis){

  clipHelper(vertices, resultingList, properties, resultingProperties, axis, 0.00313f);
  vertices.clear();
  properties.clear();

  if(resultingList.empty()){
    return 0;
  }

  clipHelper(resultingList, vertices, resultingProperties, properties, axis, -0.00313f);
  resultingList.clear();
  resultingProperties.clear();
  // vertices = resultingList;
  // properties = resultingProperties;
  // // cout << "GETS HERE " << !vertices.empty() << endl;
  return !vertices.empty();
}


void transformToClippingSpace(vector<vec4>& vertices, Camera cam){
  // for(int i = 0; i < vertices.size(); i++){
  //   vertices[i].w = 1;
  //   vertices[i] = CLIP_SPACE_TRANSFORM * vertices[i];
  // }
  for(size_t i = 0; i < vertices.size(); i++){
    // vertices[i].w = vertices[i].z / cam.focalLength;
    vertices[i].w = vertices[i].z / SCREEN_HEIGHT;
  }
}

void perspectiveDivide(vector<Vertex>& vertices){
  for(size_t i = 0; i < vertices.size(); i++){
    vertices[i].position.x = vertices[i].position.x / vertices[i].position.w;
    vertices[i].position.y = vertices[i].position.y / vertices[i].position.w;
    vertices[i].position.z = vertices[i].position.z / vertices[i].position.w;
    vertices[i].position.w = 1;
  }
}


void Draw(screen* screen, vector<Triangle>& triangles, vector<RenderedObject>& objects, Camera cam, Light light)
{

  // for(int i = 0; i < SCREEN_HEIGHT; i++){
  //     for(int j = 0; j < SCREEN_WIDTH; j++){
  //       currentPixels[i][j] = vec3(0, 0, 0);
  //     }
  // }
  currentMaximumX = -numeric_limits<int>::max(); 
  currentMinimumX = numeric_limits<int>::max();;
  currentMaximumY = -numeric_limits<int>::max();
  currentMinimumY = numeric_limits<int>::max();;
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
  float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH];
  if(doAntiAliasing){
    for(int y = 0; y < SCREEN_HEIGHT; y++){
      for(int x = 0; x < SCREEN_WIDTH; x++){
        depthBuffer[y][x] = 0.0f;
        shadowMap[y][x] = 0.0f;
        currentPixels[y][x] = zero;
      }
    }
  }else{
    for(int y = 0; y < SCREEN_HEIGHT; y++){
      for(int x = 0; x < SCREEN_WIDTH; x++){
        depthBuffer[y][x] = 0.0f;
        shadowMap[y][x] = 0.0f;
      }
    }
  }
  Camera lightCamera;
  // reset_camera(lightPOVCam);
  // lightPOVCam.cameraPos = vec4(light.lightPos.x, light.lightPos.y, light.lightPos.z, 1);
  lightCamera = getLightCamera(light);
  if(doShadows){
    for(uint32_t i = 0; i < triangles.size(); i++){
      vector<Vertex> vertices(3);
      vertices[0].position = triangles[i].v0;
      vertices[1].position = triangles[i].v1;
      vertices[2].position = triangles[i].v2;

      DrawShadowMapForTriangle(screen, vertices, light, lightCamera, shadowMap);
    }
    for(uint32_t i = 0; i < objects.size(); i++){
        for(uint32_t j = 0; j < objects[i].triangles.size(); j++){
        vector<Vertex> vertices(3);
        vertices[0].position = objects[i].triangles[j].v0;
        vertices[1].position = objects[i].triangles[j].v1;
        vertices[2].position = objects[i].triangles[j].v2;
        DrawShadowMapForTriangle(screen, vertices, light, lightCamera, shadowMap);
        }
    }
  }

  // #pragma omp parallel for
  for(size_t i = 0; i < triangles.size(); i++){
    vec4 currentNormal = triangles[i].normal;
    vec3 currentReflectance(1,1,1);

    vector<Vertex> vertices(3);
    vertices[0].position = triangles[i].v0;
    vertices[1].position = triangles[i].v1;
    vertices[2].position = triangles[i].v2;

    vertices[0].texturePosition = triangles[i].uv0;
    vertices[1].texturePosition = triangles[i].uv1;
    vertices[2].texturePosition = triangles[i].uv2;

    // // -------------------

    // Clipping should happen here!

    DrawPolygon(screen, vertices, cam, triangles[i].color, depthBuffer, shadowMap, light, lightCamera, currentNormal, currentReflectance, triangles[i].hasTexture, NULL);
    }

    for(uint32_t i = 0; i < objects.size(); i++){
      for(uint32_t j = 0; j < objects[i].triangles.size(); j++){
      vec4 currentNormal = objects[i].triangles[j].normal;
      vec3 currentReflectance(1,1,1);

      vector<Vertex> vertices(3);
      vertices[0].position = objects[i].triangles[j].v0;
      vertices[1].position = objects[i].triangles[j].v1;
      vertices[2].position = objects[i].triangles[j].v2;

      vertices[0].texturePosition = objects[i].triangles[j].uv0;
      vertices[1].texturePosition = objects[i].triangles[j].uv1;
      vertices[2].texturePosition = objects[i].triangles[j].uv2;

      DrawPolygon(screen, vertices, cam, objects[i].triangles[j].color, depthBuffer, shadowMap, light, lightCamera, currentNormal, currentReflectance, objects[i].triangles[j].hasTexture, objects[i].texture_surface);
      }
    }

    if(doAntiAliasing){
      for(int i = currentMinimumX; i < currentMaximumX; i++){
        for(int j = currentMinimumY; j < currentMaximumY; j++){
          PutPixelSDL(screen, i, j, doAntiAliasing ? getAliasedPixel(currentPixels, i, j) : currentPixels[i][j]);
        }
      }
    }
}

void Update(Camera &cam, Light &light)
{
  // static int t = SDL_GetTicks();
  // // /* Compute frame time */
  // int t2 = SDL_GetTicks();
  // float dt = float(t2-t);
  // t = t2;

  // cout << "FPS: " << 1000 / dt << endl;
  // // /* Update variables*/

  const uint8_t *keyState = SDL_GetKeyboardState(NULL);
  int lastRecordedX = 0;
  int lastRecordedY = 0;
  SDL_GetRelativeMouseState(&lastRecordedX, &lastRecordedY);
  if( keyState[SDL_SCANCODE_W] ){
    cam.cameraPos += cam.cameraRotationX * cam.cameraRotationY * vec4(0, 0, 0.02f, 0);
  }
  if( keyState[SDL_SCANCODE_S] ){
    cam.cameraPos -= cam.cameraRotationX * cam.cameraRotationY * vec4(0, 0, 0.02f, 0);
  }
  if( keyState[SDL_SCANCODE_A] ){
    cam.cameraPos -= cam.cameraRotationX * cam.cameraRotationY * vec4(0.02f, 0, 0, 0);
  }
  if( keyState[SDL_SCANCODE_D] ){
    cam.cameraPos += cam.cameraRotationX * cam.cameraRotationY * vec4(0.02f, 0, 0, 0);
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
    light.lightPos -= vec3(0,0.02f, 0);  
    }
    if( keyState[SDL_SCANCODE_BACKSLASH] ){
    light.lightPos += vec3(0,0.02f,0);  
    }
    if( keyState[SDL_SCANCODE_H] ){
      doAntiAliasing = 1;
    }
    if( keyState[SDL_SCANCODE_J] ){
      doAntiAliasing = 0;
    }
    if( keyState[SDL_SCANCODE_B] ){
      if(!doShadows)
        reset_light(light, 1);
      doShadows = 1;
    }
    if( keyState[SDL_SCANCODE_N] ){
      if(doShadows)
        reset_light(light, 0);
      doShadows = 0;
    }
    if( keyState[SDL_SCANCODE_U] ){
      doClipping = 1;
    }
    if( keyState[SDL_SCANCODE_I] ){
      doClipping = 0;
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
    reset_light(light, doShadows);
  }
}
