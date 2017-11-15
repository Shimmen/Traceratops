#ifndef TRACERATOPS_SCENE_H
#define TRACERATOPS_SCENE_H

#include <vector>

#include "geometry.h"
#include "material.h"
#include "texture.h"

class scene
{
public:

    scene();
    ~scene() = default;

    int register_material(const material& Material);
    const material& get_material(int Material) const;

    //void set_environment_map(std::unique_ptr<texture> EnvironmentMap);
    //const texture *get_environment_map() const;

    //
    // TODO: Implement a proper scene acceleration structure!
    // int register_renderable(const renderable& Renderable);
    // (or similar)
    //

    std::vector<plane> Planes;
    std::vector<sphere> Spheres;

    // (sphere map)
    std::unique_ptr<texture> EnvironmentMap;
    float EnvironmentMultiplier;

private:

    std::vector<material> RegisteredMaterials;

};

#endif // TRACERATOPS_SCENE_H
