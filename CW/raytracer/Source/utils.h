#ifndef UTILS
#define UTILS
#include "sphere.h"
bool checkBoundingBox(float x, float y);

void orthonormalSystem(const vec4& N, vec3& Nt, vec3& Nb);

void reset_evolutionModel(vec3** pixels, int& samplesSeenSoFar);

bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1);

void normaliseTriangles(vector<Triangle>& rawTriangles, float scale,
                        float displacementX, float displacementY, float displacementZ,
                        float rotateX, float rotateY, float rotateZ,
                        float adjustNormalX, float adjustNormalY, float adjustNormalZ,
                        const char* type);

#endif
