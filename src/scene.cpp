#include "scene.h"

using namespace tracemath;

scene::scene()
{
    // Register default material (index 0)
    material DefaultMaterial{vec3{0, 0, 0}, 1.0, 0.0};
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
