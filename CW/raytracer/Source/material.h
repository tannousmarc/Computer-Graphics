#ifndef MATERIAL
#define MATERIAL


struct Material{
  float emission;
  const char* type;

  Material() = default;
  Material(float emission, const char* type):
    emission(emission), type(type) {};
};

#endif
