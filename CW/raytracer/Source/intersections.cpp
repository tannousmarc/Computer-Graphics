#include "intersections.h"
#include "definitions.h"

bool ClosestIntersection( Camera& cam, vec4 dir,
    const vector<Triangle>& triangles,
    Intersection& closestIntersection ){

  float minimumDistance = std::numeric_limits<float>::max();
  bool okay = false;

  for(unsigned int i = 0; i < triangles.size(); i++){
    Triangle triangle = triangles[i];
    vec4 v0 = triangle.v0;
    vec4 v1 = triangle.v1;
    vec4 v2 = triangle.v2;
    vec3 e1 = vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
    vec3 e2 = vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
    vec3 b = vec3(cam.cameraPos.x-v0.x,cam.cameraPos.y-v0.y,cam.cameraPos.z-v0.z);
    mat3 A( vec3(-dir), e1, e2 );
    mat3 m = A;
    m[0] = b;
    float t = glm::determinant(m) / glm::determinant(A);

    if(t >= 0 && t <= minimumDistance){
      vec3 x = glm::inverse( A ) * b;
      if(x.y >= 0 && x.z >= 0 && (x.y + x.z <= 1.001)){
        minimumDistance = abs(x.x);
        closestIntersection.position = v0 + x.y*vec4(e1.x, e1.y, e1.z, 0) + x.z*vec4(e2.x, e2.y, e2.z, 0);
        closestIntersection.distance = abs(x.x);
        closestIntersection.triangleIndex = i;
        okay = true;
      }
    }
  }

  return okay;
};

bool ClosestIntersectionLight( vec4 start, vec4 dir,
    const vector<Triangle>& triangles, Intersection inter){
  bool okay = false;

  for(unsigned int i = 0; i < triangles.size(); i++){
    if((int) i == inter.triangleIndex)
      continue;
    Triangle triangle = triangles[i];
    vec4 v0 = triangle.v0;
    vec4 v1 = triangle.v1;
    vec4 v2 = triangle.v2;
    vec3 e1 = vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
    vec3 e2 = vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
    vec3 b = vec3(start.x-v0.x,start.y-v0.y,start.z-v0.z);
    mat3 A( vec3(-dir), e1, e2 );

    mat3 m = A;
    m[0] = b;
    float t = glm::determinant(m) / glm::determinant(A);
    if(t >= 0 && t <= inter.distance){
      vec3 x = glm::inverse( A ) * b;
      if(x.y >= 0 && x.z >= 0 && (x.y + x.z <= 1)){
        okay = true;
        break;
      }
    }
  }

  return okay;
};
