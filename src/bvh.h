#ifndef TRACERATOPS_BVH_H
#define TRACERATOPS_BVH_H

#include "geometry.h"

struct bvh_node: public hitable
{
    bvh_node(hitable **Hitables, size_t Count);
    ~bvh_node();

    virtual bool intersect(const ray& Ray, float TMin, float TMax, hit_info& Hit) const;
    virtual bool get_aabb(aabb& AABB) const;

    aabb NodeAABB;

    const hitable *Left;
    const hitable *Right;

};

bvh_node assemble_bvh_recursive(const std::vector<hitable *>& Hitables);

#endif //TRACERATOPS_BVH_H
