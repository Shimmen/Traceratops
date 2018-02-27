#include "bvh.h"

static int aabb_compare(const void *L, const void *R)
{
    hitable *Left = *(hitable **)L;
    hitable *Right = *(hitable **)R;

    aabb LeftAABB, RightAABB;
    if(!Left->get_aabb(LeftAABB) || !Right->get_aabb(RightAABB))
    {
        printf("Trying to assemble a BVH for a hitable without a bounding box!\n");
    }

    // Compare along a random axis
    auto CompareAxis = int(rng::global_rng().random_01() * 3.0f);
    if (LeftAABB.Min[CompareAxis] - RightAABB.Min[CompareAxis] < 0.0f)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

bvh_node::bvh_node(hitable **Hitables, size_t Count)
{
    assert(Count != 0);

    qsort(Hitables, Count, sizeof(hitable *), aabb_compare);

    if (Count == 1)
    {
        Left = Hitables[0];
        Right = Hitables[0];
    }
    else if (Count == 2)
    {
        Left = Hitables[0];
        Right = Hitables[1];
    }
    else
    {
        size_t LeftSize = Count / 2;
        Left = new bvh_node(Hitables, LeftSize);
        Right = new bvh_node(Hitables + LeftSize, Count - LeftSize);
    }

    aabb AABBs[2];
    if(!Left->get_aabb(AABBs[0]) || !Right->get_aabb(AABBs[1]))
    {
        printf("Trying to assemble a BVH for a hitable without a bounding box!\n");
    }
    NodeAABB = aabb_enclosing(AABBs, 2);
}

bool bvh_node::intersect(const ray& Ray, float TMin, float TMax, hit_info& Hit) const
{
    if (aabb_ray_intersection(NodeAABB, Ray.Direction, Ray.Origin))
    {
        hit_info LeftHitInfo{}, RightHitInfo{};
        bool LeftHit = Left->intersect(Ray, TMin, TMax, LeftHitInfo);
        bool RightHit = Right->intersect(Ray, TMin, TMax, RightHitInfo);

        if (LeftHit && RightHit)
        {
            if (LeftHitInfo.Distance < RightHitInfo.Distance)
            {
                Hit = LeftHitInfo;
            }
            else
            {
                Hit = RightHitInfo;
            }
            return true;
        }

        if (LeftHit)
        {
            Hit = LeftHitInfo;
            return true;
        }

        if (RightHit)
        {
            Hit = RightHitInfo;
            return true;
        }

    }

    return false;
}

bool bvh_node::get_aabb(aabb& AABB) const
{
    AABB = NodeAABB;
    return true;
}

