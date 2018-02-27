#include "aabb.h"
#include "scene.h"
#include "tracemath.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

using namespace tracemath;

scene::scene()
{
    // Register default material (index 0)
    lambertian *DefaultMaterial = new lambertian{vec3{1, 0, 1}, 1.0f};
    register_material(DefaultMaterial);
}

scene::~scene()
{
    for (const material *Material: RegisteredMaterials)
    {
        delete Material;
    }

    delete BVHRootNode;
}

int
scene::register_material(const material *Material)
{
    size_t NextIndex = RegisteredMaterials.size();
    RegisteredMaterials.push_back(Material);
    return static_cast<int>(NextIndex);
}

const material&
scene::get_material(int Material) const
{
    assert(Material >= 0 && Material < RegisteredMaterials.size());
    assert(RegisteredMaterials[Material]);
    return *RegisteredMaterials[Material];
}

void
scene::register_triangle_mesh(const std::string& ObjFileName, const vec3& Translation)
{
    delete BVHRootNode;
    BVHRootNode = nullptr;

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

const std::vector<vec3>&
scene::get_triangle_vertices() const
{
    return TriangleVertices;
}

void
scene::prepare_for_rendering()
{
    printf("Preparing scene ...\n");

    hitable **HitablesArray = Hitables.data();
    size_t  HitablesCount = Hitables.size();
    BVHRootNode = new bvh_node(HitablesArray, HitablesCount);

    printf("... done preparing scene\n");
}

bool
scene::is_prepared_for_rendering() const
{
    return BVHRootNode != nullptr;
}
