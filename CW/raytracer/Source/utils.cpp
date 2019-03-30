#include "utils.h"
#include "definitions.h"

 
bool checkBoundingBox(float x, float y){
  return x >= 0  && y >= 0 && x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT;
}

// draw uniform samples on a plane and project them to the hemisphere
vec3 diffuseHemisphere(float a, float b){
  const float r = sqrt(1.0f - a * a);
  const float phi = 2 * 3.14 * b;
  return vec3(cos(phi) * r, a, sin(phi) * r);
}

void orthonormalSystem(const vec4& N, vec3& Nt, vec3& Nb){
  if (std::fabs(N.x) > std::fabs(N.y))
    Nt = vec3(N.z, 0, -N.x) / sqrtf(N.x * N.x + N.z * N.z);
  else
    Nt = vec3(0, -N.z, N.y) / sqrtf(N.y * N.y + N.z * N.z);
  Nb = cross(vec3(N.x, N.y, N.z), Nt);
}

void reset_evolutionModel(vec3** pixels, int& samplesSeenSoFar){
  for(int x = 0; x < SCREEN_WIDTH; x++){
    for(int y = 0; y < SCREEN_HEIGHT; y++){
      pixels[x][y] = vec3(0.f, 0.f, 0.f);
    }
  }

  samplesSeenSoFar = 0;
}

bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = - 0.5 * b / a;
    else {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) std::swap(x0, x1);

    return true;
}

// SCALE/ROTATE OBJECTS
void normaliseTriangles(vector<Triangle>& rawTriangles, float scale = 1,
                        float displacementX = 0, float displacementY = 0, float displacementZ = 0,
                        float rotateX = 0, float rotateY = 0, float rotateZ = 0,
                        float adjustNormalX = 1, float adjustNormalY = 1, float adjustNormalZ = 1,
                        char* type = "diffuse"){
  float L = -numeric_limits<int>::max();
  for(auto triangle : rawTriangles){
    if(glm::abs(triangle.v0.x) > L){
      L = glm::abs(triangle.v0.x);
    }
    if(glm::abs(triangle.v0.y) > L){
      L = glm::abs(triangle.v0.y);
    }
    if(glm::abs(triangle.v0.z) > L){
      L = glm::abs(triangle.v0.z);
    }

    if(glm::abs(triangle.v1.x) > L){
      L = glm::abs(triangle.v1.x);
    }
    if(glm::abs(triangle.v1.y) > L){
      L = glm::abs(triangle.v1.y);
    }
    if(glm::abs(triangle.v1.z) > L){
      L = glm::abs(triangle.v1.z);
    }

    if(glm::abs(triangle.v2.x) > L){
      L = glm::abs(triangle.v2.x);
    }
    if(glm::abs(triangle.v2.y) > L){
      L = glm::abs(triangle.v2.y);
    }
    if(glm::abs(triangle.v2.z) > L){
      L = glm::abs(triangle.v2.z);
    }
  }


  vec4 v1(1, 0, 0, 0);
  vec4 v2(0, cos(rotateX), sin(rotateX),0);
  vec4 v3(0, -sin(rotateX), cos(rotateX), 0);
  vec4 v4(0, 0, 0, 1);

  mat4 Rx(v1, v2, v3, v4);

  vec4 v5(cos(rotateY), 0, -sin(rotateY), 0);
  vec4 v6(0,1,0,0);
  vec4 v7(sin(rotateY), 0, cos(rotateY), 0);
  vec4 v8(0, 0, 0, 1);

  mat4 Ry(v5, v6, v7, v8);

  vec4 v9(cos(rotateZ), sin(rotateZ), 0, 0);
  vec4 v10(-sin(rotateZ),cos(rotateZ),0,0);
  vec4 v11(0, 0, 1, 0);
  vec4 v12(0, 0, 0, 1);

  mat4 Rz(v9, v10, v11, v12);
  mat4 rotationMatrix = Rx * Ry * Rz;

  for(auto &triangle : rawTriangles){
    triangle.v0 /= scale * L;
		triangle.v1 /= scale * L;
		triangle.v2 /= scale * L;

		triangle.v0 += vec4(displacementX, displacementY, displacementZ,0);
		triangle.v1 += vec4(displacementX, displacementY, displacementZ,0);
    triangle.v2 += vec4(displacementX, displacementY, displacementZ,0);


		triangle.v0 = rotationMatrix * triangle.v0;
    triangle.v1 = rotationMatrix * triangle.v1;
    triangle.v2 = rotationMatrix * triangle.v2;
		// triangle.v2.x *= -1;

		// triangle.v0.y *= -1;
		// triangle.v1.y *= -1;
		// triangle.v2.y *= -1;
    if(!triangle.normal.x){
		  triangle.ComputeNormal();
    }
    triangle.normal.x = adjustNormalX * triangle.normal.x;
    triangle.normal.y = adjustNormalY * triangle.normal.y;
    triangle.normal.z = adjustNormalZ * triangle.normal.z;
    triangle.normal = rotationMatrix * triangle.normal;
    triangle.material.type = type;
  }
}
