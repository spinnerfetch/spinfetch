#ifndef RENDERER_H_
#define RENDERER_H_

#define GRID_HEIGHT 32
#define GRID_WIDTH 48

#define CAMERA_DISTANCE 7.0
#define FOCAL_LENGTH 30.0

#define LIGHT_DIRECTION (Vector3){-0.408248f, 0.408248f, -0.816497f}

#define FOG_RANGE 2.0
#define FOG_STRENGTH 0.4
#define JITTER_STRENGTH 0.07
#define SHADE_RAMP ".:;'^-=j?+*ckm%8#B@$" 

#include "pointcloud.h"

typedef struct {
    float x;
    float y;
    float z;
} ScreenPoint;


typedef struct {
    int r;
    int g;
    int b;
    char character;
} Cell;

typedef struct {
    Cell cells[GRID_HEIGHT][GRID_WIDTH];
    float depth[GRID_HEIGHT][GRID_WIDTH];
} Frame;


void initFrame(Frame *frame);
char *rowText(Frame *frame, int y);

float pseudoRandom(int seed);

float computeFitScale(PointCloud cloud, float targetRadius);
Frame renderFrame(PointCloud cloud, float alpha, float beta, float gamma, float scale);

#endif