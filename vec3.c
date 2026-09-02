#include <math.h>
#include "vec3.h"

// Adds two Vector3's
Vector3 addVec(Vector3 a, Vector3 b) {
    return (Vector3){a.x + b.x, a.y + b.y, a.z+b.z};
}


// Subtracts two Vector3's
Vector3 subVec(Vector3 a, Vector3 b) {
    return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z};
}


// Multiplies a Vector3 by a scalar
Vector3 mulVec(Vector3 v, float scalar) {
    return (Vector3){v.x * scalar, v.y * scalar, v.z * scalar};
}


// Divides a Vector3 by a scalar
Vector3 divVec(Vector3 v, float scalar) {
    return (Vector3){v.x / scalar, v.y / scalar, v.z / scalar};
}


// Returns the magnitude of a Vector3
float getVectorLength(Vector3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}


// Calculates the alignment between two Vector3's
float dotVec(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}


Vector3 crossVec(Vector3 a, Vector3 b) {
    return (Vector3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}


// Normalizes a Vector3
Vector3 normalizeVector(Vector3 v) {
    float length = getVectorLength(v);
    if(length == 0) {
        return (Vector3){0, 0, 0};
    }
    return (Vector3){v.x/length, v.y/length, v.z/length};
} 


// Rotates a Vector3 along three angles alpha, beta and gamma
Vector3 rotatePoint(Vector3 point, float alpha, float beta, float gamma) {
    Vector3 rotatedPoint;

    rotatedPoint.x = point.y * sin(alpha) * sin(beta) * cos(gamma) - point.z * cos(alpha) * sin(beta) * cos(gamma) + point.y * cos(alpha) * sin(gamma) + point.z * sin(alpha) * sin(gamma) + point.x * cos(beta) * cos(gamma);
    rotatedPoint.y = point.y * cos(alpha) * cos(gamma) + point.z * sin(alpha) * cos(gamma) - point.y * sin(alpha) * sin(beta) * sin(gamma) + point.z * cos(alpha) * sin(beta) * sin(gamma) - point.x * cos(beta) * sin(gamma);
    rotatedPoint.z = point.z * cos(alpha) * cos(beta) - point.y * sin(alpha) * cos(beta) + point.x * sin(beta);

    return rotatedPoint;
}
