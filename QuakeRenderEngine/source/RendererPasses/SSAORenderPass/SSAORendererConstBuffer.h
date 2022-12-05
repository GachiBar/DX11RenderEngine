#ifdef __cplusplus
#pragma once
#include "TransformUtils.h"
#include "CoreRenderSystem/CoreShaderInclude.h"

//enum TAADebugFlags {
//	MarkNoHistoryPixels = 1,
//	AllowDepthThreshold = 2,
//	AllowBicubicFilter = 4,
//	AllowVarianceClipping = 8,
//	AllowYCoCg = 16,
//	AllowNeighbourhoodSampling = 32,
//	AllowLongestVelocityVector = 64,
//};

#endif

#define SSAO_NUM_THREADS_X 1
#define SSAOBLUR_NUM_THREADS 8
#define SSAO_NUM_THREADS_Y 1



struct SSAOCosntBuffer
{
    float4x4 gProjTex;//+++
    float4   gOffsetVectors[14];//+++
    

    float2 gInvRenderTargetSize; // 1/width, 1/ height

    // Coordinates given in view space.
    float    gOcclusionRadius;
    float    gOcclusionFadeStart;
    float    gOcclusionFadeEnd;
    float    gSurfaceEpsilon;

    
    float sigma;
    float intensity;
    //int radius;
    int  kernel;//++
};

STRUCTURE(1, SSAOCosntBuffer, SSAOData)