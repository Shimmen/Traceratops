#ifndef TRACERATOPS_SCENE_H
#define TRACERATOPS_SCENE_H

#include "geometry.h"

typedef struct scene
{
    int NumPlanes;
    plane *Planes;

    int NumSpheres;
    sphere *Spheres;

    // TODO: Remove for proper lights later
    vec3 LightDirection;

} scene;

scene scene_make(int NumSpheres, int NumPlanes);
void scene_release(scene *Scene);

#endif // TRACERATOPS_SCENE_H
