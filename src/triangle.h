#ifndef TRACERATOPS_TRIANGLE_MESH_H
#define TRACERATOPS_TRIANGLE_MESH_H

#include "tracemath.h"

struct triangle_face
{
    vec3 Vertices[3];

    bool HasNormals = false;
    vec3 Normals[3];

    bool HasUVs = false;
    vec2 UVs[3];
};

#endif // TRACERATOPS_TRIANGLE_MESH_H
