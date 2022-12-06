#ifndef _CORE_SHADER_RG_
#define _CORE_SHADER_RG_
#ifndef HLSL
#pragma once
#include "TransformUtils.h"
// COMMON
struct CONST_HANDLE {
	CONST_HANDLE(int slot) { this->slot = slot; }
	int slot;
};

struct TEXTURE_HANDLE {
	TEXTURE_HANDLE(int slot) { this->slot = slot; }
	int slot;
};
#else

#ifdef GACHI
#include "../DX11RenderEngine/DX11RenderEngine/Shaders/PixelPacking_Velocity.hlsl"
#else
#include "ref_dx11rg/DX11RenderEngine/DX11RenderEngine/Shaders/PixelPacking_Velocity.hlsl"
#endif

#endif


#ifdef HLSL
#define STRUCTURE(slot, typeName, name) cbuffer __buffer##slot : register(b##slot) { typeName name : packoffset(c0); }
#else
#define STRUCTURE(slot, typeName, name) const CONST_HANDLE name(slot);
#endif

#ifdef HLSL
#define TEXTURE_2D(slot, type, name) Texture2D<type> name : register(t##slot);
#else
#define TEXTURE_2D(slot, type, typeName, name) const TEXTURE_HANDLE  name(slot);
#endif

#ifndef HLSL
#pragma pack(4)
#endif

struct ViewProjectionMatrices {
	matrix view;
	matrix projection;
	matrix inverseView;
	matrix inverseProjection;
	matrix viewProjection;
	matrix viewProjectionInverse;
	matrix reprojectionMatrix;
};

struct CoreConstants {

 
    ViewProjectionMatrices currentMatrices;
    ViewProjectionMatrices pastMatrices;

	float4 viewPosition;

    struct {
        float2 taaPixelShift;
        float taaStrength;
    } taaBuffer;

    float time;
    float dt;
};

#ifndef HLSL
#pragma pack()
#endif



STRUCTURE(0, CoreConstants, coreConstants)



/*
// HEADER

struct CAMERA {
	float4x4 view;
	float4x4 proj;
	float3   pos;
}

struct MODEL {
	float4 world;
}

STRUCTURE(0, CAMERA, camera)
STRUCTURE(1, MODEL, model)
TEXTURE_2D(0, float4, colorTexture)

// HLSL

p = mul(camera.view, v);
c = colorTexture.Sample(...);

// C++

CAMERA camera = /* .../
constBuffer->SetData(camera);
SetConstBuffer(camera.slot, constBuffer);

SetShaderResource(colorTexture.slot, /* ... /);

*/
#endif
