#include "aabb.h"
#include "scene.h"
#include "tracemath.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

using namespace tracemath;

scene::scene()
{
    // Register default material (index 0)
    //material DefaultMaterial{vec3{1, 0, 1}, 1.0, 1.0};
    material DefaultMaterial{vec3{1, 1, 1}, 0.25, 0.0};
    register_material(DefaultMaterial);
}

int
scene::register_material(const material& Material)
{
    size_t NextIndex = RegisteredMaterials.size();
    RegisteredMaterials.push_back(Material);
    return static_cast<int>(NextIndex);
}

const material&
scene::get_material(int Material) const
{
    assert(Material >= 0 && Material < RegisteredMaterials.size());
    return RegisteredMaterials[Material];
}

void
scene::register_triangle_mesh(const std::string& ObjFileName, const tracemath::vec3& Translation)
{
    BVHUpToDate = false;

    tinyobj::attrib_t Attributes;
    std::vector<tinyobj::shape_t> Shapes;
    std::vector<tinyobj::material_t> Materials;

    // TODO/FIXME!
    const char * MtlBase = "assets/";

    std::string Error;
    bool Success = tinyobj::LoadObj(&Attributes, &Shapes, &Materials, &Error, ObjFileName.c_str(), MtlBase, true);

    if (!Error.empty())
    {
        printf("Failed to load obj file '%s': %s!\n", ObjFileName.c_str(), Error.c_str());
    }

    if (!Success)
    {
        return;
    }

    for (auto& Material: Materials)
    {
        // TODO: Register materials!!!
    }

    // Add shapes as separate triangle meshes
    for (auto& Shape: Shapes)
    {
        triangle_mesh Mesh = {};

        size_t NextVertexIndex = TriangleVertices.size();
        Mesh.FirstVertex = static_cast<int>(NextVertexIndex);

        // TODO: Maybe include this?!
        //Mesh.BoundingBox.Min = vec3{ +std::numeric_limits<float>::infinity() };
        //Mesh.BoundingBox.Max = vec3{ -std::numeric_limits<float>::infinity() };

        /////////////////////////////////////////////////////////////////////////

        // Loop over faces (should only be triangles)
        for (size_t FaceIndex = 0; FaceIndex < Shape.mesh.num_face_vertices.size(); ++FaceIndex)
        {
            assert(Shape.mesh.num_face_vertices[FaceIndex] == 3);

            triangle_face TriangleFace = {};

            // Loop over the vertices in the face
            for (size_t VertexIndex = 0; VertexIndex < 3; ++VertexIndex)
            {
                tinyobj::index_t idx = Shape.mesh.indices[Mesh.VertexCount + VertexIndex];

                // TODO: Later, transform vertices & normals with the specified transform!

                vec3 Vertex{
                    Attributes.vertices[3 * idx.vertex_index + 0] + Translation.x,
                    Attributes.vertices[3 * idx.vertex_index + 1] + Translation.y,
                    Attributes.vertices[3 * idx.vertex_index + 2] + Translation.z
                };

                TriangleVertices.push_back(Vertex);

                // Update the bounding box for the current mesh
                //min(&Mesh.BoundingBox.Min, Vertex);
                //max(&Mesh.BoundingBox.Max, Vertex);

                TriangleFace.Vertices[VertexIndex] = Vertex;

                TriangleFace.Normals[VertexIndex] = vec3{
                    Attributes.normals[3 * idx.normal_index + 0],
                    Attributes.normals[3 * idx.normal_index + 1],
                    Attributes.normals[3 * idx.normal_index + 2]
                };

                if (Attributes.texcoords.size() >= 2 * idx.texcoord_index)
                {
                    TriangleFace.UVs[VertexIndex] = vec2{
                        Attributes.texcoords[2 * idx.texcoord_index + 0],
                        Attributes.texcoords[2 * idx.texcoord_index + 1]
                    };
                }
                else
                {
                    TriangleFace.UVs[VertexIndex] = vec2{
                        -1.0f, -1.0f
                    };
                }
            }

            // TODO: (per-face) materials ... (Shape.mesh.material_ids[Face];)
            // For now though, use default material (=0)
            TriangleFace.Material = 0;

            TriangleFaces.push_back(TriangleFace);
            Mesh.VertexCount += 3;
        }

        /////////////////////////////////////////////////////////////////////////

        TriangleMeshes.push_back(Mesh);
    }
}

const std::vector<tracemath::vec3>&
scene::get_triangle_vertices() const
{
    return TriangleVertices;
}

void
scene::prepare_for_rendering()
{
    printf("Preparing scene ...\n");

    // Assemble a BVH (bounding volume hierarchy), top down

    constexpr float Infinity = std::numeric_limits<float>::infinity();
    vec3 CurrentMin = vec3{ +Infinity };
    vec3 CurrentMax = vec3{ -Infinity };

    // Define BVH root
    for (const vec3& Vertex: TriangleVertices)
    {
        max(&CurrentMax, Vertex);
        min(&CurrentMin, Vertex);
    }

    BVHElements.emplace_back(CurrentMin, CurrentMax);
    for (const triangle_face& Triangle: TriangleFaces)
    {
        BVHElements[0].ContainedTriangles.push_back(&Triangle);
    }

    const int LeafTriangleMaxCount = 5;
    const int MaxTreeDepth = 6;
    assemble_bvh(LeafTriangleMaxCount, MaxTreeDepth);

    BVHUpToDate = true;
    printf("... done preparing scene\n");

}

bool
scene::is_prepared_for_rendering() const
{
    return BVHUpToDate;
}

void
scene::assemble_bvh(int LeafTriangleMaxCount, int MaxDepth)
{
    std::vector<std::pair<int, int>> Stack{};

    auto Root = std::make_pair(0, 0);
    Stack.push_back(Root);

    while (!Stack.empty())
    {
        const std::pair<int, int>& CurrentElement = Stack.back();
        Stack.pop_back();

        // If we have reached the max depth in this node, don't add any children
        int CurrentDepth = CurrentElement.second;
        if (CurrentDepth == MaxDepth - 1)
        {
            continue;
        }

        int Current = CurrentElement.first;

        // Split the current AABB in half by the longest axis
        const vec3 CurrentMax = BVHElements[Current].Max;
        const vec3 CurrentMin = BVHElements[Current].Min;
        const vec3 Diff = CurrentMax - CurrentMin;
        const vec3 LongestAxisDiff = max_and_zeroes(Diff);
        const int IndexOfMax = index_of_max(Diff);

        // Calculate max corner of "left" part
        vec3 LeftMaxDiff = Diff;
        LeftMaxDiff[IndexOfMax] = 0.5f * LeftMaxDiff[IndexOfMax];
        vec3 LeftMax = CurrentMin + LeftMaxDiff;

        // Calculate min corner of "right" part
        vec3 OffsetToMin = LongestAxisDiff;
        OffsetToMin[IndexOfMax] = 0.5f * OffsetToMin[IndexOfMax];
        vec3 RightMin = CurrentMin + OffsetToMin;

        aabb LeftChild = aabb{CurrentMin, LeftMax};
        aabb RightChild = aabb{RightMin, CurrentMax};

        // TODO: --------------------------------------------------------------------------- :TODO
        // TODO: -- Is this really a BVH? Since it doesn't try to shrink the child AABBs? -- :TODO
        // TODO: --------------------------------------------------------------------------- :TODO
        //               (it's more like an octree, but with two children instead of 8)

        //
        // Check which triangles are contained in which part
        //

        for (const triangle_face *Triangle: BVHElements[Current].ContainedTriangles)
        {
            constexpr float Infinity = std::numeric_limits<float>::infinity();
            vec3 Min = vec3{ +Infinity };
            vec3 Max = vec3{ -Infinity };

            for (const vec3& Vertex: Triangle->Vertices)
            {
                max(&Max, Vertex);
                min(&Min, Vertex);
            }

            aabb TriangleAABB{ Min, Max };

            bool InLeft  = aabb_intersection(LeftChild, TriangleAABB);
            bool InRight = aabb_intersection(RightChild, TriangleAABB);

            if (InLeft) LeftChild.ContainedTriangles.push_back(Triangle);
            if (InRight) RightChild.ContainedTriangles.push_back(Triangle);
        }

        //
        // Only create children if they contain at least one triangle!
        // Don't create sub-volumes if they contain less triangles than LeafTriangleMaxCount!
        //

        size_t CurrentCount = BVHElements[Current].ContainedTriangles.size();

        size_t LeftCount = LeftChild.ContainedTriangles.size();
        if (LeftCount > 0 && LeftCount < CurrentCount)
        {
            size_t LeftChildIndex = BVHElements.size();
            BVHElements.push_back(LeftChild);
            BVHElements[Current].Children.push_back(LeftChildIndex);

            if (LeftCount > LeafTriangleMaxCount)
            {

                Stack.emplace_back(LeftChildIndex, CurrentDepth + 1);
            }
        }

        size_t RightCount = RightChild.ContainedTriangles.size();
        if (RightCount > 0 && RightCount < CurrentCount)
        {
            size_t RightChildIndex = BVHElements.size();
            BVHElements.push_back(RightChild);
            BVHElements[Current].Children.push_back(RightChildIndex);

            if (RightCount > LeafTriangleMaxCount)
            {
                Stack.emplace_back(RightChildIndex, CurrentDepth + 1);
            }
        }
    }

    BVHUpToDate = true;
}
