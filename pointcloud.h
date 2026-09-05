#ifndef POINTCLOUD_H_
#define POINTCLOUD_H_

#include "vec3.h"


typedef struct {
    float r;
    float g;
    float b;
} Color;


typedef struct {
    Vector3 position;
    Vector3 normal;
    Color color;
} Point;


typedef struct {
    Point *points;
    size_t count;
} PointCloud;


PointCloud loadPointcloud(const char *filename);
void centerPointcloud(PointCloud cloud);
void normalizePointcloudScale(PointCloud cloud, float targetRadius);
void savePointcloud(PointCloud cloud, const char *path);

#endif
