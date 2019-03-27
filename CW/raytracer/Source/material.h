#ifndef MATERIAL
#define MATERIAL


struct Material{
  const char* type;

  Material() = default;
  Material(const char* type):
    type(type) {};
};

#endif
