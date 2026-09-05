#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "renderer.h"
#include "vec3.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

void initFrame(Frame *frame) {
    for(int y = 0; y < GRID_HEIGHT; y++){
        for(int x = 0; x < GRID_WIDTH; x++){
            frame->cells[y][x].r = 0;
            frame->cells[y][x].g = 0;
            frame->cells[y][x].b = 0;
            frame->cells[y][x].character = ' ';

            frame->depth[y][x] = INFINITY;
        }
    }
}


char *rowText(Frame *frame, int y) {
    size_t capacity = (GRID_WIDTH + 5) * 32 + 1;
    char *row = malloc(capacity);
    if(row == NULL) return NULL;

    row[0] = '\0';

    for(int x = 0; x < GRID_WIDTH; x++){
        Cell cell = frame->cells[y][x];
        char cellText[32];
        snprintf(cellText, sizeof(cellText), "\033[1;38;2;%d;%d;%dm%c", cell.r, cell.g, cell.b, cell.character);
        strcat(row, cellText);
    }
    strcat(row, "\033[0m");
    return row;
}


float pseudoRandom(int seed) {
    float x = sinf(seed * 12.9898) * 43758.5453;
    return x - floorf(x);
}


ScreenPoint projectPoint(Vector3 point) {
    float zCam = point.z + CAMERA_DISTANCE;
    float projectedX = point.x * FOCAL_LENGTH / zCam * 2;
    float projectedY = point.y * FOCAL_LENGTH / zCam;
    return (ScreenPoint){projectedX, projectedY, zCam};
}


float computeFitScale(PointCloud cloud, float targetRadius) {
    float maxDist = 0.0f;

    for(size_t p = 0; p < cloud.count; p++){
        Point point = cloud.points[p];

        ScreenPoint screen = projectPoint(point.position);
        float distance = sqrtf(screen.x * screen.x + screen.y * screen.y);

        if(distance > maxDist) maxDist = distance;
    }

    if(maxDist == 0.0f) return 1.0f;
    return (float)(targetRadius / maxDist);
}


Frame renderFrame(PointCloud cloud, float alpha, float beta, float gamma, float scale) {
    Frame frame;
    initFrame(&frame);

    int halfWidth = GRID_WIDTH / 2;
    int halfHeight = GRID_HEIGHT / 2;

    for(size_t pointIndex = 0; pointIndex < cloud.count; pointIndex++){
        Point point = cloud.points[pointIndex];

        Vector3 rotatedPosition = rotatePoint(point.position, alpha, beta, gamma);
        Vector3 rotatedNormals = rotatePoint(point.normal, alpha, beta, gamma);
        ScreenPoint screenPoint = projectPoint(rotatedPosition);

        int screenX = (int)((screenPoint.x * scale)) + halfWidth;
        int screenY = (int)((screenPoint.y * scale)) + halfHeight;

        if(!(0 <= screenX && screenX < GRID_WIDTH && 0 <= screenY && screenY < GRID_HEIGHT)) continue;
        if(screenPoint.z >= frame.depth[screenY][screenX]) continue;

        float rawDot = dotVec(rotatedNormals, LIGHT_DIRECTION);
        float brightness = rawDot * 0.5 + 0.5;
        if(brightness < 0.0f) brightness = 0.0f;
        if(brightness > 1.0f) brightness = 1.0f;

        float fog = (screenPoint.z - (CAMERA_DISTANCE - FOG_RANGE)) / (2 * FOG_RANGE);
        if(fog < 0.0f) fog = 0.0f;
        if(fog > 1.0f) fog = 1.0f;

        float charBrightness = brightness * (1 - fog * FOG_STRENGTH);

        float jitter = (pseudoRandom(pointIndex) - 0.5) * JITTER_STRENGTH;
        charBrightness += jitter;
        if(charBrightness < 0) charBrightness = 0;
        if(charBrightness > 1) charBrightness = 1;

        int charIndex = (int)(charBrightness * (strlen(SHADE_RAMP) - 1));
        char character = SHADE_RAMP[charIndex];

        Color color = point.color;
        int r = min(255, (int)(color.r * (1.6 * brightness) * (1 - fog * FOG_STRENGTH)));
        int g = min(255, (int)(color.g * (1.6 * brightness) * (1 - fog * FOG_STRENGTH)));
        int b = min(255, (int)(color.b * (1.6 * brightness) * (1 - fog * FOG_STRENGTH)));

        frame.cells[screenY][screenX].r = r;
        frame.cells[screenY][screenX].g = g;
        frame.cells[screenY][screenX].b = b;
        frame.cells[screenY][screenX].character = character;

        frame.depth[screenY][screenX] = screenPoint.z;
    }
    return frame;
}