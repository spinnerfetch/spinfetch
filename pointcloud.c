#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec3.h"
#include "pointcloud.h"


PointCloud loadPointcloud(const char *path) {
    FILE *file = fopen(path, "r");
    char line[512];

    if(file == NULL){
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    if(fgets(line, sizeof(line), file) == NULL){
        fprintf(stderr, "Could not read SPC file %s\n", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    line[strcspn(line, "\r\n")] = '\0';

    if(strcmp(line, "SPC1") != 0){
        fprintf(stderr, "%s is not a valid SPC file\n", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if(fgets(line, sizeof(line), file) == NULL){
        fprintf(stderr, "Missing point count in file %s\n", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t count;
    if(sscanf(line, "%zu", &count) != 1){
        fprintf(stderr, "Invalid point count\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if(count == 0){
        fprintf(stderr, "Point cloud in file %s is empty\n", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    Point *points = malloc(count * sizeof(Point));
    if(points == 0){
        fprintf(stderr, "Could not allocate memory to points\n");
        free(points);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    for(size_t pointIndex = 0; pointIndex < count; pointIndex++){
        if(fgets(line, sizeof(line), file) == NULL){
            fprintf(stderr, "Unexpected end of file %s at point %zu\n", path, pointIndex);
            free(points);
            fclose(file);
            exit(EXIT_FAILURE);
        }
        
        float posX, posY, posZ;
        float normX, normY, normZ;
        float r, g, b;

        if(sscanf(line, "%f %f %f %f %f %f %f %f %f", &posX, &posY, &posZ, &normX, &normY, &normZ, &r, &g, &b) != 9){
            fprintf(stderr, "Point %zu in file %s is invalid\n", pointIndex, path);
            free(points);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        Vector3 position = {posX, posY, posZ};
        Vector3 normal = {normX, normY, normZ};
        Color color = {r, g, b};

        points[pointIndex] = (Point){position, normal, color};
    }
    fclose(file);

    PointCloud cloud = {points, count};
    centerPointcloud(cloud);
    return cloud;
}


void centerPointcloud(PointCloud cloud) {
    if(cloud.count == 0) {
        fprintf(stderr, "Point cloud is empty\n");
        exit(EXIT_FAILURE);
    }

    float minX = cloud.points[0].position.x;
    float minY = cloud.points[0].position.y;
    float minZ = cloud.points[0].position.z;
    float maxX = minX;
    float maxY = minY;
    float maxZ = minZ;

    for(size_t i = 0; i < cloud.count; i++){
        Point *point = &cloud.points[i];
        Vector3 pos = (*point).position;

        if(pos.x < minX) {
            minX = pos.x;
        }
        if(pos.y < minY) {
            minY = pos.y;
        }
        if(pos.z < minZ) {
            minZ = pos.z;
        }
        if(pos.x > maxX) {
            maxX = pos.x;
        }
        if(pos.y > maxY) {
            maxY = pos.y;
        }
        if(pos.z > maxZ) {
            maxZ = pos.z;
        }
    }
    float centerX = (minX + maxX) / 2;
    float centerY = (minY + maxY) / 2;
    float centerZ = (minZ + maxZ) / 2;

    for(size_t i = 0; i < cloud.count; i++){
        Point *point = &cloud.points[i];

        point->position.x -= centerX;
        point->position.y -= centerY;
        point->position.z -= centerZ;
    }
}


void savePointcloud(PointCloud cloud, const char *path) {
    FILE *file = fopen(path, "w");

    if(file == NULL) {
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "SPC1\n");
    fprintf(file, "%zu\n", cloud.count);
    for(size_t i = 0; i < cloud.count; i++) {
        Point *point = &cloud.points[i];

        Vector3 pos = (*point).position;
        Vector3 norm = (*point).normal;
        Color color = (*point).color;

        fprintf(file, "%f %f %f %f %f %f %f %f %f\n", pos.x, pos.y, pos.z, norm.x, norm.y, norm.z, color.r, color.g, color.b);
    }
    fclose(file);
}
