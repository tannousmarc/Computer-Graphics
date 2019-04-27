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
int clipTriangleAxis(vector<Vertex>& vertices, vector<Vertex>& resultingList, int axis);



const float f = 10;
// const float n = SCREEN_HEIGHT;
const float n = 2;
const float t = 1;
const float r = 1;
const float l = -1;
const float b = -1;

float CLIPPING_ARR[16] = {
        2 * n / (r - l) , 0               , 0                      , 0  ,
        0               , 2 * n / (t - b) , 0                      , 0  ,
        (r + l)/(r - l) , (t + b)/(t - b) , - (f + n)/(f - n)      , -1 ,
        0               , 0               , -(2 * f * n) / (f - n) , 0
};


// float CLIPPING_ARR[16] = {
//         SCREEN_WIDTH/2 , 0               , 0                      , SCREEN_WIDTH/2  ,
//         0               , -SCREEN_HEIGHT/2 , 0                      , SCREEN_HEIGHT/2  ,
//         0 , 0 , 1      , 0 ,
//         0               , 0               , 0 , 1
// };
mat4 CLIP_SPACE_TRANSFORM;



vec3 currentPixels[SCREEN_HEIGHT][SCREEN_WIDTH];
int currentMinimumX = 0, currentMinimumY = 0;
int currentMaximumX = SCREEN_HEIGHT, currentMaximumY = SCREEN_WIDTH;
int doAntiAliasing = 0;
int doShadows = 0;
int doClipping = 0;
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

inline void VertexShaderClipping(const Vertex& v, Pixel& p, Camera cam){
  // vec4 vNew = (v.position - cam.cameraPos) * (cam.cameraRotationX * cam.cameraRotationY);
  vec4 vNew = v.position;
  // p.pos3d = vec3(v.position.x, v.position.y, v.position.z);
  p.pos3d = prevPos3d;

  // if(vNew.z < 0.001)
  //   return;

  // vNew.w = v.position.w * (cam.cameraRotationX * cam.cameraRotationY);
  // cout << cam.focalLength << " " << vNew.z << " " << vNew.w << " " << v.position.w << " A: " << vNew.x / v.position.w + (SCREEN_WIDTH / 2) << " B: " <<  (cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2) << endl;
  // p.x = (int) ((cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2));
  p.x = (int) ((vNew.x / vNew.w) + (SCREEN_WIDTH / 2));
  // p.y = (int) ((cam.focalLength * vNew.y / vNew.z)  + (SCREEN_HEIGHT / 2));
  p.y = (int) ((vNew.y / vNew.w) + (SCREEN_HEIGHT) / 2);

  // IMPARTE LA vNew.w
  p.zinv = (float) (1.0f / vNew.z);
  // p.zinv = (float) 1.0f / (cam.focalLength * vNew.w);
  
  p.textureX = v.texturePosition.x;
  p.textureY = v.texturePosition.y;
}

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

 void DrawPolygon(screen *screen, vector<Vertex>& vertices, Camera cam, vec3 color, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH], Light light, Camera lightCamera, vec4 normal, vec3 reflectance, bool hasTexture, SDL_Surface* texture_surface, bool afterClipping){
   int V = vertices.size();
   vector<Pixel> vertexPixels(V);

   if(!afterClipping){
    for(int i = 0; i < V; i++){
      VertexShader(vertices[i], vertexPixels[i], cam);
    }
   }else{
     for(int i = 0; i < V; i++){
      VertexShaderClipping(vertices[i], vertexPixels[i], cam);
    }
   }

  
    if(doClipping && !afterClipping){
      vector<vec4> positions;
      for(int i = 0; i < V; i++){
        vertices[i].position = (vertices[i].position - cam.cameraPos) * (cam.cameraRotationX * cam.cameraRotationY);
      }

      positions.push_back(vertices[0].position);
      positions.push_back(vertices[1].position);
      positions.push_back(vertices[2].position);
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

      // cout << "Positions: "<< positions[0].x << " " << positions[0].y << " " << positions[0].z << " " << positions[0].w << endl
      //   << positions[1].x << " " << positions[1].y << " " << positions[1].z << " " << positions[1].w << endl
      //   << positions[2].x << " " << positions[2].y << " " << positions[2].z << " " << positions[2].w << endl;
      // cout << "IN DRAW" << endl;

      if(clipTriangleAxis(vertices, resultingVertices, 0)){
          cout << vertices.size() << endl;
          vector<Vertex> currentTriangleVertices(3);
            for(size_t v = 1; v < vertices.size()-1; v++){
              currentTriangleVertices[0] = vertices[0];
              currentTriangleVertices[1] = vertices[v];
              currentTriangleVertices[2] = vertices[v+1];
              cout << "AFTER ONE: " << currentTriangleVertices[0].position.x << " " << currentTriangleVertices[0].position.y << " " << currentTriangleVertices[0].position.z << " " << currentTriangleVertices[0].position.w << " " << endl;
              cout << "AFTER TWO: " << currentTriangleVertices[v].position.x << " " << currentTriangleVertices[v].position.y << " " << currentTriangleVertices[v].position.z << " " << currentTriangleVertices[v].position.w << " " <<endl;
              cout << "AFTER THREE: " << currentTriangleVertices[v+1].position.x << " " << currentTriangleVertices[v+1].position.y << " " << currentTriangleVertices[v+1].position.z << " " << currentTriangleVertices[v+1].position.w << " " <<endl;

              cout << "SHOULD BE DRAWING?" << endl;
              DrawPolygon(screen, currentTriangleVertices, cam, color,
                depthBuffer, shadowMap, light, lightCamera, normal, reflectance, hasTexture, NULL, 1);
            }
          // }
        }
        return;
    }


   vector<Pixel> leftPixels;
   vector<Pixel> rightPixels;

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

  //  ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
  //  DrawPolygonRows(screen, leftPixels, rightPixels, color, depthBuffer, light, normal, reflectance);
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

float getPositionByAxis(Vertex v, int axis){
  if(axis == 0) return v.position.x;
  if(axis == 1) return v.position.y;
  if(axis == 2) return v.position.z;
  return v.position.w;
}

void clipHelper(vector<Vertex> &vertices, vector<Vertex> &resultingVertices,
                  int axis, float factor){
  Vertex previousVertex = vertices[vertices.size() - 1];
  float previousComponent = getPositionByAxis(previousVertex, axis) * factor;
  unsigned int previousInside = (previousComponent <= previousVertex.position.w);
  for(size_t i = 0; i < vertices.size(); i++){
    Vertex currentVertex = vertices[i];
    float currentComponent = getPositionByAxis(currentVertex, axis) * factor;
    int currentInside = currentComponent <= currentVertex.position.w;
    
    if(currentInside ^ previousInside){
      float interpolateAmount = (previousVertex.position.w - previousComponent) /
        ((previousVertex.position.w - previousComponent) -
         (currentVertex.position.w - currentComponent));
      resultingVertices.push_back(interpolate(previousVertex, currentVertex, interpolateAmount));
    }

    if(currentInside){
      resultingVertices.push_back(currentVertex);
    }
    previousVertex = currentVertex;
    previousComponent = currentComponent;
    previousInside = currentInside;
    // float current
  }
};


int clipTriangleAxis(vector<Vertex>& vertices, vector<Vertex>& resultingList, int axis){

  clipHelper(vertices, resultingList, axis, 1.0f);
  vertices.clear();

  if(resultingList.empty()){
    return 0;
  }

  clipHelper(resultingList, vertices, axis, -1.0f);
  resultingList.clear();

  // cout << "GETS HERE " << !vertices.empty() << endl;
  return !vertices.empty();
}


void transformToClippingSpace(vector<vec4>& vertices, Camera cam){
  // for(int i = 0; i < vertices.size(); i++){
  //   vertices[i].w = 1;
  //   vertices[i] = CLIP_SPACE_TRANSFORM * vertices[i];
  // }
  for(size_t i = 0; i < vertices.size(); i++){
    vertices[i].w = vertices[i].z / cam.focalLength;
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

    DrawPolygon(screen, vertices, cam, triangles[i].color, depthBuffer, shadowMap, light, lightCamera, currentNormal, currentReflectance, triangles[i].hasTexture, NULL, 0);
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

      DrawPolygon(screen, vertices, cam, objects[i].triangles[j].color, depthBuffer, shadowMap, light, lightCamera, currentNormal, currentReflectance, objects[i].triangles[j].hasTexture, objects[i].texture_surface, 0);
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
