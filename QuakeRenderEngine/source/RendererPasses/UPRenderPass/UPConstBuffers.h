#ifdef __cplusplus
#pragma once
#include "TransformUtils.h"
#include "CoreRenderSystem/CoreShaderInclude.h"
#endif


struct UPCosntBuffer {
    matrix world;
    float4 color;
    float2 texOffset;
};

STRUCTURE(1, UPCosntBuffer, upCosntBuffer)