#ifdef __cplusplus
#pragma once
#include "TransformUtils.h"
#include "CoreRenderSystem/CoreShaderInclude.h"
#endif

struct OpaqueCosntBuffer {
    matrix world;
    matrix oldWorld;

    float3 normal;
    float roughness;
    float4 diffuse;
    float metallic;
    
    float MetallicMult;
    float RoughnessMult;
    uint  id;
    
};

STRUCTURE(1, OpaqueCosntBuffer, opaqueCosntBuffer)