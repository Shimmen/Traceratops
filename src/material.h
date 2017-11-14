#ifndef TRACERATOPS_MATERIAL_H
#define TRACERATOPS_MATERIAL_H

#include "tracemath.h"

struct material
{
    float Roughness;
    float Emittance;

    tracemath::vec3 Albedo;

    material(tracemath::vec3 Albedo, float Roughness, float Emittance = 0.0f)
        : Albedo(Albedo), Roughness(Roughness), Emittance(Emittance) {}
};

#endif // TRACERATOPS_MATERIAL_H
