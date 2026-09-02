#ifndef VEC3_H_
#define VEC3_H_

// Definition of Vector3
typedef struct {
    float x;
    float y;
    float z;
} Vector3;


Vector3 addVec(Vector3 a, Vector3 b);
Vector3 subVec(Vector3 a, Vector3 b);
Vector3 mulVec(Vector3 v, float scalar);
Vector3 divVec(Vector3 v, float scalar);

float getVectorLength(Vector3 v);
float dotVec(Vector3 a, Vector3 b);
Vector3 crossVec(Vector3 a, Vector3 b);
Vector3 normalizeVector(Vector3 v);
Vector3 rotatePoint(Vector3 v, float alpha, float beta, float gamma);

#endif
