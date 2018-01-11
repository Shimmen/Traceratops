#ifndef TRACERATOPS_SCENE_H
#define TRACERATOPS_SCENE_H

#include <vector>
#include <memory>

#include "aabb.h"
#include "texture.h"
#include "geometry.h"
#include "material.h"
#include "triangle_mesh.h"


class scene
{
public:

    scene();
    virtual ~scene();

    int register_material(const material *Material);
    const material& get_material(int Material) const;

    void register_triangle_mesh(const std::string& ObjFileName, const tracemath::vec3& Translation);
    const std::vector<tracemath::vec3>& get_triangle_vertices() const;

    void prepare_for_rendering();
    bool is_prepared_for_rendering() const;

    std::vector<disc> Discs;
    std::vector<plane> Planes;
    std::vector<sphere> Spheres;

    // (sphere map)
    std::unique_ptr<texture> EnvironmentMap;
    float EnvironmentMultiplier;

//private:

    void assemble_bvh(int LeafTriangleMaxCount, int MaxDepth);

    bool BVHUpToDate = false;
    std::vector<aabb> BVHElements;

    std::vector<const material *> RegisteredMaterials;

    std::vector<tracemath::vec3> TriangleVertices;
    std::vector<triangle_face>   TriangleFaces;
    std::vector<triangle_mesh>   TriangleMeshes;

};

#endif // TRACERATOPS_SCENE_H
