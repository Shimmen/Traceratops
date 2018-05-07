#ifndef TRACERATOPS_SCENE_H
#define TRACERATOPS_SCENE_H

#include <vector>
#include <memory>

#include "aabb.h"
#include "texture.h"
#include "geometry.h"
#include "material.h"
#include "triangle.h"
#include "bvh.h"

class scene
{
public:

    static const scene& get_current() { return *CurrentScene; };

    scene();
    virtual ~scene();

    int register_material(const material *Material);
    const material& get_material(int Material) const;

    void add_hitable(hitable *Hitable);
    void register_triangle_mesh(const std::string& Directory, const std::string& ObjFileName, const vec3& Translation, float Scale = 1.0f, float yRotation = 0.0f);

    void prepare_for_rendering();
    bool is_prepared_for_rendering() const;

    std::vector<hitable *> EmittingHitables;
    std::vector<hitable *> Hitables;
    bvh_node *BVHRootNode = nullptr;

    // (sphere map)
    std::unique_ptr<texture> EnvironmentMap;
    float EnvironmentMultiplier;

    std::vector<const material *> RegisteredMaterials;

    std::vector<triangle_face> TriangleFaces;

private:

    static scene *CurrentScene;

    void invalidate_current_bvh();

};

#endif // TRACERATOPS_SCENE_H
