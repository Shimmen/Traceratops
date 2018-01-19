#ifndef TRACERATOPS_TRIANGLE_MESH_H
#define TRACERATOPS_TRIANGLE_MESH_H

#include "tracemath.h"

struct triangle_face
{
    vec3 Vertices[3];
    vec3 Normals[3];
    vec2 UVs[3];

    int Material;
};

struct triangle_mesh
{
    int FirstVertex;
    int VertexCount;

    // TODO: Should I have this?
    //tracemath::aabb BoundingBox;
};

#endif // TRACERATOPS_TRIANGLE_MESH_H
