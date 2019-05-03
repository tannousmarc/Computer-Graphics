#include <iostream>
#include <glm/glm.hpp>
#include "utils.cpp"
#include <stdint.h>
#include <omp.h>
#include "FXAA.cpp"
#include <glm/gtc/type_ptr.hpp>

#define OMP_NUM_THREADS 8


void transformToClippingSpace(vector<vec4>& vertices, Camera cam);
int clipTriangleOnNumberedAxis(vector<Vertex>& vertices, vector<Vertex>& resultingList, vector<VertexProperties> &properties, vector<VertexProperties> &resultingProperties, int axis);
void BarycentricDrawPixels(vector<Pixel>& vertexPixels, vec3 color, screen* screen, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH], Light light, Camera lightCamera, vec4 normal, vec3 reflectance, int hasTexture, SDL_Surface* texture_surface);

vec3 currentPixels[SCREEN_HEIGHT][SCREEN_WIDTH];
int currentMinimumX = 0, currentMinimumY = 0;
int currentMaximumX = SCREEN_HEIGHT, currentMaximumY = SCREEN_WIDTH;
int doAntiAliasing = 0;
int doShadows = 0;
int doClipping = 0;
float clippingFactor = 0.00313f / ((float)SCREEN_HEIGHT / 640.0);

vec3 zero(0, 0, 0);

// Load normal scene
void LoadInitialScene(vector<Triangle> &triangles, vector<RenderedObject>& objects){
  triangles.clear();
  objects.clear();
  LoadTestModel(triangles);
  for(size_t i = 0; i < triangles.size(); i++){
    triangles[i].hasTexture = false;
  }
}

// Load temple and dog scene
void LoadTempleAndDog(vector<Triangle> &triangles, vector<RenderedObject>& objects){
  triangles.clear();
  objects.clear();
  vec3 white(  0.75f, 0.75f, 0.75f );
  float L = 555;
  vec4 A(L,0,0,1);
	vec4 B(0,0,0,1);
	vec4 C(L,0,L,1);
	vec4 D(0,0,L,1);

	vec4 E(L,L,0,1);
	vec4 F(0,L,0,1);
	vec4 G(L,L,L,1);
	vec4 H(0,L,L,1);

	// Floor:
	triangles.push_back( Triangle( C, B, A, white ) );
	triangles.push_back( Triangle( C, D, B, white ) );

	// Left wall
	triangles.push_back( Triangle( A, E, C, white ) );
	triangles.push_back( Triangle( C, E, G, white ) );

	// Right wall
	triangles.push_back( Triangle( F, B, D, white ) );
	triangles.push_back( Triangle( H, F, D, white ) );

	// // // Ceiling
	triangles.push_back( Triangle( E, F, G, white ) );
	triangles.push_back( Triangle( F, H, G, white ) );

	// Back wall
	triangles.push_back( Triangle( G, D, C, white ) );
	triangles.push_back( Triangle( G, H, D, white ) );

  for( size_t i=0; i<triangles.size(); ++i )
	{
		triangles[i].v0 *= 2/L;
		triangles[i].v1 *= 2/L;
		triangles[i].v2 *= 2/L;

		triangles[i].v0 -= vec4(1,1,1,1);
		triangles[i].v1 -= vec4(1,1,1,1);
		triangles[i].v2 -= vec4(1,1,1,1);

		triangles[i].v0.x *= -1;
		triangles[i].v1.x *= -1;
		triangles[i].v2.x *= -1;

		triangles[i].v0.y *= -1;
		triangles[i].v1.y *= -1;
		triangles[i].v2.y *= -1;

		triangles[i].v0.w = 1.0;
		triangles[i].v1.w = 1.0;
		triangles[i].v2.w = 1.0;
		
		triangles[i].ComputeNormal();
    triangles[i].hasTexture = false;
	}

  RenderedObject temple = LoadObject("Objects/Temple/temple.obj");
  normaliseTriangles(temple.triangles,
                     0.7, 
                     -0.3, -0.7, 0.1,
                     3.1, 0, 0,
                     1, -1, -1);
  temple.texture_surface = SDL_LoadBMP("Objects/Temple/temple.bmp");
  objects.push_back(temple);

  RenderedObject dog = LoadObject("Objects/DogMaya/dog.obj");
  normaliseTriangles(dog.triangles,
                     1.6, 
                     0.4, -0.6, 0.0,
                     1.8f, 0, 0,
                     1, -1, -1);
  dog.texture_surface = SDL_LoadBMP("Objects/DogMaya/Dog_diffuse.bmp");
  objects.push_back(dog);

}

Camera getLightCamera(Light light){

   Camera lightCamera;
   lightCamera.cameraRotationY = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
   lightCamera.cameraRotationX = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
   lightCamera.cameraRotationZ = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
   lightCamera.yaw = 0.0;
   lightCamera.pitch = 0.0;
   lightCamera.focalLength = SCREEN_HEIGHT/2;
   lightCamera.cameraPos = vec4(light.lightPos.x, light.lightPos.y, light.lightPos.z, 1);
   return lightCamera;
}

inline void VertexShader(const Vertex& v, Pixel& p, Camera cam){
  vec4 vNew = (v.position - cam.cameraPos) * (cam.cameraRotationX * cam.cameraRotationY);
  p.pos3d = vec3(v.position.x, v.position.y, v.position.z);
  p.x = (int) ((cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2));
  p.y = (int) ((cam.focalLength * vNew.y / vNew.z)  + (SCREEN_HEIGHT / 2));
  p.zinv = (float) (1.0f / vNew.z);
  p.textureX = v.texturePosition.x;
  p.textureY = v.texturePosition.y;
}

// The vertex shader used in the clipping procedure
void VertexShaderClipping(const Vertex& v, VertexProperties &props, Pixel &p){
  p.pos3d = props.initialPosition;
  p.x = (v.position.x / v.position.w) + (SCREEN_WIDTH / 2);
  p.y = (v.position.y / v.position.w) + (SCREEN_HEIGHT / 2);
  p.zinv = props.zinv;
  p.textureX = props.texturePos.x;
  p.textureY = props.texturePos.y;
}

// Keep track of the initial properties of the vertices, as to use later on
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

  if(p.zinv > depthBuffer[p.y][p.x]){
    depthBuffer[p.y][p.x] = p.zinv;

    if(p.x >= 0 && p.x < SCREEN_WIDTH && p.y >= 0 && p.y < SCREEN_HEIGHT){
      // Simply put pixel on screen if anti aliasing is not activated, otherwise put it in an image matrix 
      if(!doAntiAliasing){
        PutPixelSDL(screen, p.x, p.y, illumination * color);
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

  if(p.zinv > depthBuffer[p.y][p.x]){
    depthBuffer[p.y][p.x] = p.zinv;
    

    if(p.x >= 0 && p.x < SCREEN_HEIGHT && p.y >= 0 && p.y < SCREEN_WIDTH){

      vec3 toPutPixel = illumination*color;
      if(doShadows){
        if((cameraPixel.zinv < shadowMap[cameraPixel.y][cameraPixel.x] && abs(shadowMap[cameraPixel.y][cameraPixel.x] - cameraPixel.zinv) > 0.01f)){
          toPutPixel.x /= 2;
          toPutPixel.y /= 2;
          toPutPixel.z /= 2;
        }
      }

      if(!doAntiAliasing){
        
        PutPixelSDL(screen, p.x, p.y, toPutPixel);
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

 void DrawPolygon(screen *screen, vector<Vertex>& vertices, Camera cam, vec3 color, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH], Light light, Camera lightCamera, vec4 normal, vec3 reflectance, bool hasTexture, SDL_Surface* texture_surface){
   int V = vertices.size();
   vector<Pixel> vertexPixels(V);
     
  if(doClipping){
      // Clipping routine
      vector<vec4> positions(3);
      vector<VertexProperties> properties(3);
      
      // Remember initial properties of the vertices as to use later on
      for(int i = 0; i < V; i++){
        RememberPropertiesClipping(vertices[i], properties[i], cam);
        positions[i] = ((vertices[i].position - cam.cameraPos) * (cam.cameraRotationX * cam.cameraRotationY));
      }
      
      // Transform to the vec4 positions to the clip space
      transformToClippingSpace(positions, cam);
      
      vertices[0].position = positions[0];
      vertices[1].position = positions[1];
      vertices[2].position = positions[2];

      vector<Vertex> resultingVertices;
      vector<VertexProperties> resultingProperties;

      // If clipping on all of the axes doesn't return a 0 (i.e. the vertices list is not empty)
      if(clipTriangleOnNumberedAxis(vertices, resultingVertices, properties, resultingProperties, 0) &&
      clipTriangleOnNumberedAxis(vertices, resultingVertices, properties, resultingProperties, 1)){

         vector<Pixel> currentPixels(3);
         vector<Pixel> pixels(vertices.size());

         // Call special vertex shader
         for(size_t v = 0; v < vertices.size(); v++){
            VertexShaderClipping(vertices[v], properties[v], pixels[v]);
         }

          for(size_t v = 1; v < vertices.size()-1; v++){

              currentPixels[0] = pixels[0];
              currentPixels[1] = pixels[v];
              currentPixels[2] = pixels[v+1];
              
              // Draw pixels
              BarycentricDrawPixels(currentPixels, color, screen, depthBuffer, shadowMap, light, lightCamera, normal, reflectance, hasTexture, texture_surface);
            }
          
      }
            
      return;
  }else{

    for(int i = 0; i < V; i++){
      VertexShader(vertices[i], vertexPixels[i], cam);
    }
    // Draw pixels
    BarycentricDrawPixels(vertexPixels, color, screen, depthBuffer, shadowMap, light, lightCamera, normal, reflectance, hasTexture, texture_surface);
  }
 }


 void BarycentricDrawPixels(vector<Pixel>& vertexPixels, vec3 color, screen* screen, float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH], float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH], const Light light, const Camera lightCamera, const vec4 normal, const vec3 reflectance, const int hasTexture, SDL_Surface* texture_surface){

   int maxXX = -numeric_limits<int>::max(); 
   int minXX = numeric_limits<int>::max();;
   int maxYY = -numeric_limits<int>::max();
   int minYY = numeric_limits<int>::max();;
   
   // Find bounding points for the box
   for(size_t i = 0; i < vertexPixels.size(); i++){
     maxXX = std::max(maxXX, vertexPixels[i].x);
     maxYY = std::max(maxYY, vertexPixels[i].y);
     minXX = std::min(minXX, vertexPixels[i].x);
     minYY = std::min(minYY, vertexPixels[i].y);
   }

   const int maxX = maxXX;
   const int minX = minXX;
   const int maxY = maxYY;
   const int minY = minYY;

  
   for(int y = minY; y < maxY; y++){
     if(y >= SCREEN_HEIGHT || y < 0)
      continue;
     
    // #pragma omp parallel for
     for(int x = minX; x < maxX; x++){
       if(x < 0 || x >= SCREEN_WIDTH)
        continue;
       
       // Find corresponding points as to derive a, b and c factors
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

       // Get a, b and c
       float denom = dot00 * dot11 - dot01 * dot01;
       float a = (dot11 * dot20 - dot01 * dot21) / denom;
       float b = (dot00 * dot21 - dot01 * dot20) / denom;
       float c = 1.0f - a - b;

       if (0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1){
        tPixel.zinv = vertexPixels[1].zinv*a + vertexPixels[2].zinv*b + vertexPixels[0].zinv*c;
        tPixel.pos3d = (vertexPixels[1].pos3d * vertexPixels[1].zinv*a + vertexPixels[2].pos3d * vertexPixels[2].zinv*b + vertexPixels[0].pos3d * vertexPixels[0].zinv*c) / tPixel.zinv;
        if(hasTexture && texture_surface != NULL){
          color = getTextureAt(texture_surface, vertexPixels[1].textureX*a+vertexPixels[2].textureX*b+vertexPixels[0].textureX*c,
            vertexPixels[1].textureY*a+vertexPixels[2].textureY*b+vertexPixels[0].textureY*c);
        }

        if(doShadows){
          // If shadows are activated, use the shadow map to get colour of pixel
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
          // Otherwise, simply call pixel shader
          PixelShader(tPixel, color, screen, depthBuffer, shadowMap, light, normal, reflectance);
        }
       }
     }
   }
 }

// Vertex shader for shadow mapping
inline void VertexShaderShadow(const Vertex& v, Pixel& p, Camera cam){
    vec4 vNew = (v.position - cam.cameraPos) * cam.cameraRotationX;

    if(vNew.z < 0.00001)
      return;
    p.x = (int) ((cam.focalLength * vNew.x / vNew.z) + (SCREEN_WIDTH / 2));
    p.y = (int) ((cam.focalLength * vNew.y / vNew.z)  + (SCREEN_HEIGHT / 2));
    p.zinv = (float) (1.0f / vNew.z);
    p.pos3d = vec3(v.position.x, v.position.y, v.position.z);
    p.textureX = v.texturePosition.x;
    p.textureY = v.texturePosition.y;
  }



 // For the shadow map, simply render the image from the point of view of the light, with the shadowMap array used like the depthBuffer
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
       if (0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1){
        tPixel.zinv = vertexPixels[1].zinv*a + vertexPixels[2].zinv*b + vertexPixels[0].zinv*c;

        if((tPixel.zinv > shadowMap[tPixel.y][tPixel.x]) || shadowMap[tPixel.y][tPixel.x] == 0.0f){
          shadowMap[tPixel.y][tPixel.x] = tPixel.zinv;
        }
      }
    }
  }
 }





int main( int argc, char* argv[] )
{
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  SDL_SetRelativeMouseMode(SDL_TRUE);

  vector<Triangle> triangles;
  vector<RenderedObject> objects;

  LoadTempleAndDog(triangles, objects);
                       
  Camera cam;
  reset_camera(cam);

  Light light;
  reset_light(light, doShadows);
  while (NoQuitMessageSDL())
    {
      Update(cam, light, triangles, objects);
      Draw(screen, triangles, objects, cam, light);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

// Interpolates two properties based on the distance between the vertices represented by them
VertexProperties interpolateProperties(VertexProperties first, VertexProperties last, float amount){
  VertexProperties result;
  result.zinv = first.zinv * (1-amount) + last.zinv * amount;
  result.relativePosition = first.relativePosition * (1-amount) + last.relativePosition * amount;
  result.initialPosition = first.initialPosition * (1 - amount) + last.initialPosition * amount;
  result.texturePos.x = first.texturePos.x * (1 - amount) + last.texturePos.x * amount;
  result.texturePos.y = first.texturePos.y * (1 - amount) + last.texturePos.y * amount;
  return result;
}

// Helps in having a cleaner code
float getPositionByAxis(Vertex v, int axis){
  if(axis == 0) return v.position.x;
  if(axis == 1) return v.position.y;
  if(axis == 2) return v.position.z;
  return v.position.w;
}

// Clip the axis based on the given factor for the boundary - this function returns an array of vertices
void clipAxisBasedOnFactor(vector<Vertex> &vertices, vector<Vertex> &resultingVertices, 
  vector<VertexProperties> &properties, vector<VertexProperties> &resultingProperties, int axis, float factor){

  Vertex previousVertex = vertices[vertices.size() - 1];
  VertexProperties previousProperties = properties[properties.size() - 1];
  float previousComponent = (getPositionByAxis(previousVertex, axis)) * factor;
  unsigned int previousInside = (previousComponent <= previousVertex.position.w);
  for(size_t i = 0; i < vertices.size(); i++){
    Vertex currentVertex = vertices[i];
    VertexProperties currentProperties = properties[i];
    float currentComponent = (getPositionByAxis(currentVertex, axis)) * factor;
    int currentInside = currentComponent <= currentVertex.position.w;
    
    if(currentInside ^ previousInside){
      float interpolateAmount = (previousVertex.position.w - previousComponent) /
        ((previousVertex.position.w - previousComponent) -
         (currentVertex.position.w - currentComponent));
      
      Vertex toPushBack;
      toPushBack.position = previousVertex.position + interpolateAmount * (currentVertex.position - previousVertex.position);
      resultingVertices.push_back(toPushBack);
      resultingProperties.push_back(interpolateProperties(previousProperties, currentProperties, interpolateAmount));
    }

    if(currentInside){
      resultingVertices.push_back(currentVertex);
      resultingProperties.push_back(currentProperties);
    }
    previousVertex = currentVertex;
    previousProperties = currentProperties;
    previousComponent = currentComponent;
    previousInside = currentInside;
  }
};


// Clips given axis to the left and to the right - the result is stored in vertices
int clipTriangleOnNumberedAxis(vector<Vertex>& vertices, vector<Vertex>& resultingList, vector<VertexProperties> &properties, vector<VertexProperties> &resultingProperties, int axis){

  clipAxisBasedOnFactor(vertices, resultingList, properties, resultingProperties, axis, clippingFactor);
  vertices.clear();
  properties.clear();

  if(resultingList.empty()){
    return 0;
  }

  clipAxisBasedOnFactor(resultingList, vertices, resultingProperties, properties, axis, -clippingFactor);
  resultingList.clear();
  resultingProperties.clear();

  return !vertices.empty();
}


// Simply give w = 2*z / focal_length
void transformToClippingSpace(vector<vec4>& vertices, Camera cam){
  for(size_t i = 0; i < vertices.size(); i++){
    vertices[i].w = vertices[i].z / SCREEN_HEIGHT;
  }
}


void Draw(screen* screen, vector<Triangle>& triangles, vector<RenderedObject>& objects, Camera cam, Light light){

  currentMaximumX = -numeric_limits<int>::max(); 
  currentMinimumX = numeric_limits<int>::max();;
  currentMaximumY = -numeric_limits<int>::max();
  currentMinimumY = numeric_limits<int>::max();;
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
  float shadowMap[SCREEN_HEIGHT][SCREEN_WIDTH];
  // Initialise buffers
  if(doAntiAliasing){
    for(int y = 0; y < SCREEN_HEIGHT; y++){
      for(int x = 0; x < SCREEN_WIDTH; x++){
        depthBuffer[y][x] = 0.0f;
        currentPixels[y][x] = zero;
      }
    }
  }else{
    for(int y = 0; y < SCREEN_HEIGHT; y++){
      for(int x = 0; x < SCREEN_WIDTH; x++){
        depthBuffer[y][x] = 0.0f;
      }
    }
  }
  if(doShadows){
    for(int y = 0; y < SCREEN_HEIGHT; y++){
      for(int x = 0; x < SCREEN_WIDTH; x++){
        shadowMap[y][x] = 0.0f;
      }
    }
  }
  Camera lightCamera;
  lightCamera = getLightCamera(light);

  // If shadows are activated, first compute the shadow map
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

  // Normal triangles, without texture
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

    DrawPolygon(screen, vertices, cam, triangles[i].color, depthBuffer, shadowMap, light, lightCamera, currentNormal, currentReflectance, triangles[i].hasTexture, NULL);
    }

    // Object triangles, that have texture
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
          PutPixelSDL(screen, i, j, getAliasedPixel(currentPixels, i, j));
        }
      }
    }else{
      for(int i = currentMinimumX; i < currentMaximumX; i++){
        for(int j = currentMinimumY; j < currentMaximumY; j++){
          PutPixelSDL(screen, i, j, currentPixels[i][j]);
        }
      }
    }
}

void Update(Camera &cam, Light &light, vector<Triangle>& triangles, vector<RenderedObject>& objects)
{
  static int t = SDL_GetTicks();
  // /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;

  cout << "FPS: " << 1000 / dt << endl;
  // /* Update variables*/

  const uint8_t *keyState = SDL_GetKeyboardState(NULL);
  int lastRecordedX = 0;
  int lastRecordedY = 0;

  // Enable mouse camera
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
    cout << light.lightPos.x << " " << light.lightPos.y << " " << light.lightPos.z << endl;
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

    if( keyState[SDL_SCANCODE_T] ){
      LoadInitialScene(triangles, objects);
    }
    if( keyState[SDL_SCANCODE_Y] ){
      LoadTempleAndDog(triangles, objects);
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
