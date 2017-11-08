#ifndef TRACERATOPS_SCENE_H
#define TRACERATOPS_SCENE_H

#include <vector>

#include "geometry.h"

class scene
{
public:

    //
    // TODO: Implement a proper scene acceleation structure!
    //

    std::vector<plane> Planes;
    std::vector<sphere> Spheres;

    // TODO: Remove for proper lights later
    tracemath::vec3 LightDirection;

};

#endif // TRACERATOPS_SCENE_H
