#include <iostream>
#include <glm/glm.hpp>
#include <algorithm>
#include <mmintrin.h>

#define SUBPIXEL_QUALITY 0.75
#define EDGE_THRESHOLD_MIN 0.0312
#define EDGE_THRESHOLD_MAX 0.125
#define NO_ITERATIONS 12
using namespace std;

float minss ( float a, float b ){
    _mm_store_ss( &a, _mm_min_ss(_mm_set_ss(a),_mm_set_ss(b)) );
    return a;
}

float maxss ( float a, float b ){
    _mm_store_ss( &a, _mm_max_ss(_mm_set_ss(a),_mm_set_ss(b)) );
    return a;
}

float clamp ( float val, float minval, float maxval ){
    _mm_store_ss( &val, _mm_min_ss( _mm_max_ss(_mm_set_ss(val),_mm_set_ss(minval)), _mm_set_ss(maxval) ) );
    return val;
}

float getLuma(vec3 rgb){
    return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

float qualityVector[12]={1.5f, 1.5f, 1.5f, 2.0f, 2.0f, 2.0f, 2.0f, 4.0f, 8.0f, 8.0f, 8.0f, 8.0f};


vec3 getAliasedPixel(vec3 currentPixels[SCREEN_HEIGHT][SCREEN_WIDTH], int x, int y){
    vec3 colorCenter = currentPixels[x][y];

    if(x <= 0 || y <= 0 || x >= (SCREEN_HEIGHT - 1) || y >= (SCREEN_WIDTH - 1))
        return colorCenter;

    vec2 inverseScreenSize(1.0f/SCREEN_WIDTH, 1.0f/SCREEN_HEIGHT);

    float lumaCentre = getLuma(colorCenter);
    float lumaDown = getLuma(currentPixels[x+1][y]);
    float lumaUp = getLuma(currentPixels[x-1][y]);
    float lumaLeft = getLuma(currentPixels[x][y-1]);
    float lumaRight = getLuma(currentPixels[x][y+1]);

    float lumaMin = min(lumaCentre,min(min(lumaDown,lumaUp),min(lumaLeft,lumaRight)));
    float lumaMax = max(lumaCentre,max(max(lumaDown,lumaUp),max(lumaLeft,lumaRight)));

    float lumaRange = lumaMax - lumaMin;

    if(lumaRange < max(EDGE_THRESHOLD_MIN,lumaMax*EDGE_THRESHOLD_MAX)){
        return currentPixels[x][y];
    }

    float lumaDownUp = lumaDown + lumaUp;
    float lumaLeftRight = lumaLeft + lumaRight;

    float lumaLeftCorners = getLuma(currentPixels[x-1][y-1]) + getLuma(currentPixels[x-1][y+1]);
    float lumaDownCorners = getLuma(currentPixels[x-1][y-1]) + getLuma(currentPixels[x+1][y-1]);
    float lumaRightCorners = getLuma(currentPixels[x+1][y-1]) + getLuma(currentPixels[x+1][y+1]);
    float lumaUpCorners = getLuma(currentPixels[x+1][y+1]) + getLuma(currentPixels[x-1][y+1]);

    bool isEdgeHorizontal = (abs(-2.0f*lumaLeft+lumaLeftCorners)+abs(-2.0*lumaCentre+lumaDownUp )*2.0+abs(-2.0*lumaRight+lumaRightCorners) 
        >= abs(-2.0*lumaUp+lumaUpCorners)+abs(-2.0*lumaCentre+lumaLeftRight)*2.0+abs(-2.0*lumaDown+lumaDownCorners));

    float firstLuma = isEdgeHorizontal ? lumaDown : lumaLeft;
    float secondLuma = isEdgeHorizontal ? lumaUp : lumaRight;

    float firstGradient = firstLuma - lumaCentre;
    float secondGradient = secondLuma - lumaCentre;

    float normalizedGradient = 0.25f*max(abs(firstGradient),abs(secondGradient));
    float stepLength = isEdgeHorizontal ? inverseScreenSize.y : inverseScreenSize.x;
    float lumaLocalAverage = 0.0f;

    if(abs(firstGradient) >= abs(secondGradient)){
        stepLength = - stepLength;
        lumaLocalAverage = 0.5*(firstLuma + lumaCentre);
    } else {
        lumaLocalAverage = 0.5*(secondLuma + lumaCentre);
    }

    vec2 currentPixel(x, y);
    if(isEdgeHorizontal){
        currentPixel.y += stepLength * 0.5;
    } else {
        currentPixel.x += stepLength * 0.5;
    }

    vec2 offset = isEdgeHorizontal ? vec2(inverseScreenSize.x,0.0) : vec2(0.0,inverseScreenSize.y);
    vec2 uv1 = currentPixel - offset;
    vec2 uv2 = currentPixel + offset;

    float firstExtremityLuma = getLuma(currentPixels[(int)uv1.x][(int)uv1.y]) - lumaLocalAverage;
    float secondExtremityLuma = getLuma(currentPixels[(int)uv2.x][(int)uv2.y]) - lumaLocalAverage;
    bool gotToFirstSide = abs(firstExtremityLuma) >= normalizedGradient;
    bool gotToSecondSide = abs(secondExtremityLuma) >= normalizedGradient;
    bool gotToBothSides = gotToFirstSide && gotToSecondSide;

    if(!gotToFirstSide){
        uv1 -= offset;
    }
    if(!gotToSecondSide){
        uv2 += offset;
    }

    if(!gotToBothSides){
        for(int i = 2; i < NO_ITERATIONS; i++){
            if(!gotToFirstSide){
                firstExtremityLuma = getLuma(currentPixels[(int)uv1.x][(int)uv1.y]);
                firstExtremityLuma = firstExtremityLuma - lumaLocalAverage;
            }
            if(!gotToSecondSide){
                secondExtremityLuma = getLuma(currentPixels[(int)uv2.x][(int)uv2.y]);
                secondExtremityLuma = secondExtremityLuma - lumaLocalAverage;
            }
            gotToFirstSide = abs(firstExtremityLuma) >= normalizedGradient;
            gotToSecondSide = abs(secondExtremityLuma) >= normalizedGradient;
            gotToBothSides = gotToFirstSide && gotToSecondSide;
            if(!gotToFirstSide){
                uv1 -= offset * qualityVector[i];
            }
            if(!gotToSecondSide){
                uv2 += offset * qualityVector[i];
            }
            if(gotToBothSides){ break;}
        }
    }

    float distanceToFirstEdge = isEdgeHorizontal ? (x - uv1.x) : (y - uv1.y);
    float distanceToSecondEdge = isEdgeHorizontal ? (uv2.x - x) : (uv2.y - y);

    float finalOffset = (((distanceToFirstEdge < distanceToSecondEdge ? firstExtremityLuma : secondExtremityLuma) < 0.0) != (lumaCentre < lumaLocalAverage)) ?
        - min(distanceToFirstEdge, distanceToSecondEdge) / (distanceToFirstEdge + distanceToSecondEdge) + 0.5 : 0.0;

    float lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
    float subPixelOffset1 = clamp(abs(lumaAverage - lumaCentre)/lumaRange,0.0,1.0);
    float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
    float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

    finalOffset = max(finalOffset,subPixelOffsetFinal);

    vec2 finalPixel(x, y);
    if(isEdgeHorizontal){
        finalPixel.y += finalOffset * stepLength;
    } else {
        finalPixel.x += finalOffset * stepLength;
    }

    currentPixels[x][y] = (1.0f * currentPixels[(int)finalPixel.x][(int)finalPixel.y] + 1.0f * currentPixels[x][y]) / 2.0f;

    return currentPixels[x][y];
}

