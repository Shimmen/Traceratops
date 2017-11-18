#ifndef TRACERATOPS_TRIANGLE_MESH_H
#define TRACERATOPS_TRIANGLE_MESH_H

#include "tracemath.h"

struct triangle_face
{
    tracemath::vec3 Vertices[3];
    tracemath::vec3 Normals[3];
    tracemath::vec2 UVs[3];

    int Material;
};

struct triangle_mesh
{
    int FirstVertex;
    int VertexCount;

    tracemath::aabb BoundingBox;
};

#endif // TRACERATOPS_TRIANGLE_MESH_H
