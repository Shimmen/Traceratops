#ifndef TRACERATOPS_SCENE_H
#define TRACERATOPS_SCENE_H

#include <vector>

#include "geometry.h"
#include "material.h"

class scene
{
public:

    scene();
    ~scene() = default;

    int register_material(const material& Material);
    const material& get_material(int Material) const;

    // TODO!
    //int register_renderable(const renderable& Renderable);

    //
    // TODO: Implement a proper scene acceleation structure!
    //

    std::vector<plane> Planes;
    std::vector<sphere> Spheres;

    // TODO: Remove for proper lights later
    tracemath::vec3 LightDirection;

private:

    std::vector<material> RegisteredMaterials;

};

#endif // TRACERATOPS_SCENE_H
