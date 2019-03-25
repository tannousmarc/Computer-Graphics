#include "utils.h"
#include "definitions.h"

bool checkBoundingBox(float x, float y){
  return x >= 0  && y >= 0 && x <= SCREEN_WIDTH && y <= SCREEN_HEIGHT;
}
