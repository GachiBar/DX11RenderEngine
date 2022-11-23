#ifdef __cplusplus
#pragma once
#include "TransformUtils.h"
#include "CoreRenderSystem/CoreShaderInclude.h"
#endif


struct LightDataBuffer
{
    float4 Pos;
    float4 Dir;
    float4 Params;
    float4 Color;
};

struct LightCosntBuffer {
    matrix world;

    float4 ViewerPos;
    LightDataBuffer Light;
};

STRUCTURE(1, LightCosntBuffer, lightCosntBuffer)