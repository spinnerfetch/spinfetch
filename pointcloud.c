#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec3.h"
#include "pointcloud.h"


PointCloud loadPointcloud(const char *path) {
    // Opens file and sets line variable
    FILE *file = fopen(path, "r");
    char line[512];

    // If could not open file raise error
    if(file == NULL) {
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    // Reads first line; if can't raise error
    if(fgets(line, sizeof(line), file) == NULL) {
        fprintf(stderr, "Could not read SPC file %s\n", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Strips first line
    line[strcspn(line, "\r\n")] = '\0';

    // First line of any SPC file must be "SPC1", if not, it's not an SPC file, therefore raise error
    if(strcmp(line, "SPC1") != 0){
        fprintf(stderr, "%s is not a valid SPC file", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Second line of any SPC file is the count of points in the pointcloud. Gets stored, if invalid raise error
    fgets(line, sizeof(line), file);
    size_t count;
    if(sscanf(line, "%zu", &count) != 1){
        fprintf(stderr, "Invalid point count");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // If no points in cloud raise error
    if(count == 0){
        fprintf(stderr, "Point cloud in file %s is empty", path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Allocates memory for points, if not raise error
    Point *points = malloc(count * sizeof(Point));
    if(points == 0){
        fprintf(stderr, "Could not allocate memory to points");
        free(points);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Reads each line and creates according point
    for(size_t pointIndex = 0; pointIndex < count; pointIndex++){
        // If the loop reaches EOF before purported count, raise error
        if(fgets(line, sizeof(line), file) == NULL){
            fprintf(stderr, "Unexpected end of file %s at point %zu", pointIndex, path);
            free(points);
            fclose(file);
            exit(EXIT_FAILURE);
        }
        
        // Base variables for the Point
        float posX, posY, posZ;
        float normX, normY, normZ;
        float r, g, b;

        // Fills the variables to create a Point, if unexpected number of variables raise error
        if(sscanf(line, "%f %f %f %f %f %f %f %f %f", &posX, &posY, &posZ, &normX, &normY, &normZ, &r, &g, &b) != 9){
            fprintf(stderr, "Point %zu in file %s is invalid", pointIndex, path);
            free(points);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Fills proper structs with variables and creates proper Point in PointCloud
        Vector3 position = {posX, posY, posZ};
        Vector3 normal = {normX, normY, normZ};
        Color color = {r, g, b};

        points[pointIndex] = (Point){position, normal, color};
    }

    // Closes file, creates PointCloud, centers it and returns it
    fclose(file);

    PointCloud cloud = {points, count};
    centerPointcloud(cloud);
    return cloud;
}


void centerPointcloud(PointCloud cloud) {
    // If point cloud has no points raise error
    if(cloud.count == 0) {
        fprintf(stderr, "Point cloud is empty");
        exit(EXIT_FAILURE);
    }

    // Sets base minimals and maximals
    float minX = cloud.points[0].position.x;
    float minY = cloud.points[0].position.y;
    float minZ = cloud.points[0].position.z;
    float maxX = minX;
    float maxY = minY;
    float maxZ = minZ;

    // Finds the minimal and maximal point in each dimension by going through every point
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

    // Finds the centerpoint of the cloudpoint
    float centerX = (minX + maxX) / 2;
    float centerY = (minY + maxY) / 2;
    float centerZ = (minZ + maxZ) / 2;

    // Centers the cloudpoint in render
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
