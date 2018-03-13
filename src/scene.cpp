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
scene::add_hitable(hitable *Hitable)
{
    invalidate_current_bvh();
    Hitables.push_back(Hitable);

    if (!is_zero(get_material(Hitable->Material).EmitColor))
    {
        EmittingHitables.push_back(Hitable);
    }
}

void
scene::register_triangle_mesh(const std::string& ObjFileName, const vec3& Translation)
{
    invalidate_current_bvh();

    tinyobj::attrib_t Attributes;
    std::vector<tinyobj::shape_t> Shapes;
    std::vector<tinyobj::material_t> Materials;

    // TODO/FIXME!
    const char * MtlBase = "assets/cornell_box/";

    std::string Error;
    bool Success = tinyobj::LoadObj(&Attributes, &Shapes, &Materials, &Error, ObjFileName.c_str(), MtlBase, true);

    if (!Error.empty())
    {
        fprintf(stderr, "'%s':\n\t%s", ObjFileName.c_str(), Error.c_str());

        if (!Success)
        {
            std::exit(1);
        }
    }

    std::vector<int> MaterialIndexMap;
    for (auto& ObjMaterial: Materials)
    {
        // TODO: Do this in a better way (and support more types of materials)!
        //       Currently it's just a big hack, but we'll get there!

        // Use green emission as emission float
        float Emission = ObjMaterial.emission[1];

        vec3 DiffuseColor = vec3{ ObjMaterial.diffuse[0], ObjMaterial.diffuse[1], ObjMaterial.diffuse[2] };
        vec3 SpecularColor = vec3{ ObjMaterial.specular[0], ObjMaterial.specular[1], ObjMaterial.specular[2] };

        float DiffuseAmount = dot(DiffuseColor, vec3{1});
        float SpecularAmount = dot(SpecularColor, vec3{1});

        material *Material;
        if (DiffuseAmount > SpecularAmount)
        {
            Material = new lambertian{DiffuseColor, Emission};
        }
        else
        {
            Material = new metal{SpecularColor, 0, Emission};
        }
        int MaterialIndex = register_material(Material);

        MaterialIndexMap.push_back(MaterialIndex);
    }

    // Add shapes as separate triangle meshes
    for (auto& Shape: Shapes)
    {
        // Loop over faces (should only be triangles)
        for (size_t FaceIndex = 0; FaceIndex < Shape.mesh.num_face_vertices.size(); ++FaceIndex)
        {
            assert(Shape.mesh.num_face_vertices[FaceIndex] == 3);

            vec3 TriangleVertices[3];

            // Loop over the vertices in the face
            for (size_t VertexIndex = 0; VertexIndex < 3; ++VertexIndex)
            {
                tinyobj::index_t idx = Shape.mesh.indices[3 * FaceIndex + VertexIndex];

                // TODO: Later remove the translation part and implement proper transformations!

                TriangleVertices[VertexIndex] = vec3{
                    Attributes.vertices[3 * idx.vertex_index + 0] + Translation.x,
                    Attributes.vertices[3 * idx.vertex_index + 1] + Translation.y,
                    Attributes.vertices[3 * idx.vertex_index + 2] + Translation.z
                };
/*
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
*/
            }

            int ObjFaceMaterialIndex = Shape.mesh.material_ids[FaceIndex];
            int FaceMaterial = MaterialIndexMap[ObjFaceMaterialIndex];

            auto *Triangle = new triangle{TriangleVertices[0], TriangleVertices[1], TriangleVertices[2], FaceMaterial};
            add_hitable(Triangle);

        }

    }
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

void
scene::invalidate_current_bvh()
{
    if (BVHRootNode)
    {
        delete BVHRootNode;
        BVHRootNode = nullptr;
    }
}
