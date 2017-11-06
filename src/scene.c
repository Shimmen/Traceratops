#include "scene.h"

#include <stdlib.h>

scene scene_make(int NumSpheres, int NumPlanes)
{
    scene Scene = {};

    Scene.NumPlanes = NumPlanes;
    Scene.Planes = malloc(NumPlanes * sizeof(plane));

    Scene.NumSpheres = NumSpheres;
    Scene.Spheres = malloc(NumSpheres * sizeof(sphere));

    return Scene;
}

void scene_release(scene *Scene)
{
    if (Scene)
    {
        if (Scene->Planes)
        {
            free(Scene->Planes);
        }
        if (Scene->Spheres)
        {
            free(Scene->Spheres);
        }
    }
}
