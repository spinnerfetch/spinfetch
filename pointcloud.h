#ifndef POINTCLOUD_H_
#define POINTCLOUD_H_

#include "vec3.h"


// Definition of a Color as three values r, g and b
typedef struct {
    float r;
    float g;
    float b;
} Color;


// Definition of a Point as a position(x, y, z), normals(nx, ny, nz) and color(r, g, b)
typedef struct {
    Vector3 position;
    Vector3 normal;
    Color color;
} Point;


// Definition of a PointCloud as points and a count
typedef struct {
    Point *points;
    size_t count;
} PointCloud;


// Methods in pointcloud.c
PointCloud loadPointcloud(const char *filename);
void centerPointcloud(PointCloud cloud);
void savePointcloud(PointCloud cloud, const char *path);

#endif
