#ifndef RENDERER_H_
#define RENDERER_H_

#include "vec3.h"
#include "pointcloud.h"


#define GRID_HEIGHT 40
#define GRID_WIDTH 48
#define MARGIN_WIDTH 2

#define CAMERA_DISTANCE 3.0
#define FOCAL_LENGTH 30.0

#define SHADE_RAMP ".:-=+*?#@"

#define AMBIENT 0.3
#define BRIGHTNESS_GAMMA 0.75
#define JITTER_STRENGTH 0.15

#define RIM_STRENGTH 0.5
#define RIM_POWER 2.5

#define FOG_RANGE 2.0
#define FOG_STRENGTH 0.6

const Vector3 VIEW_DIRECTION = {0, 0, 1};
const Vector3 LIGHT_DIRECTION = {-0.408248f, 0.408248f, -0.816497f};


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

ScreenPoint projectPoint(Vector3 point, float cameraDistance, float focalLength);
float computeFitScale(PointCloud cloud, float targetRadius, float cameraDistance, float focalLength);
Frame renderFrame(PointCloud cloud, float alpha, float beta, float gamma, float scale, float cameraDistance, float focalLength);

#endif