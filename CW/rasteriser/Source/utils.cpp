#include "parser.cpp"

// CAMERA/LIGHT POSITIONS
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

// SCALE/ROTATE OBJECTS
void normaliseTriangles(vector<Triangle>& rawTriangles, float scale = 1, 
                        float displacementX = 0, float displacementY = 0, float displacementZ = 0, 
                        float rotateX = 0, float rotateY = 0, float rotateZ = 0,
                        float adjustNormalX = 1, float adjustNormalY = 1, float adjustNormalZ = 1){
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
  }
}

// GET TEXTURE PIXEL AT POSITION
vec3 getTextureAt(SDL_Surface *surface, int x, int y)
{
  int bytes_per_pixel = surface->format->BytesPerPixel;
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bytes_per_pixel;

  Uint32 sdl_pixel_value;
  vec3 pixel_value;
  switch(bytes_per_pixel) {
    case 1:
      sdl_pixel_value = *p;
      break;

    case 2:
      sdl_pixel_value = *(Uint16 *)p;
      break;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
        sdl_pixel_value = p[0] << 16 | p[1] << 8 | p[2];
      else
        sdl_pixel_value = p[0] | p[1] << 8 | p[2] << 16;

    case 4:
      sdl_pixel_value = *(Uint32 *)p;
      break;
    default:
      sdl_pixel_value = 0;     
  }
  uint8_t r, g, b;
  SDL_GetRGB(sdl_pixel_value, surface->format, &r, &g, &b);
  pixel_value.r = r/255.f;
  pixel_value.g = g/255.f;
  pixel_value.b = b/255.f;
  return pixel_value;
}