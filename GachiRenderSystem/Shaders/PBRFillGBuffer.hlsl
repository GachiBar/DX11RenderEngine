#define HLSL
#define GACHI

#include "../DX11RenderEngine/DX11RenderEngine/include/CoreRenderSystem/CoreShaderInclude.h"
#include "../DX11RenderEngine/GachiRenderSystem/source/RendererPasses/PBRPasses/OpaqueRenderPass/OpaquePassConstBuffer.h"

struct VS_IN
{
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 tex : TEXCOORD0;
	float3 binormal : BINORMAL0;
	float3 tangent : TANGENT0;
	float4 color : COLOR0;
};

//struct VS_IN_ANIM
//{
//	float4 pos : POSITION0;
//	float4 normal : NORMAL0;
//	float4 binormal : BINORMAL0;
//	float4 tangent : TANGENT0;
//	float4 color : COLOR0;
//	float4 tex : TEXCOORD0;
//	int4	inds	: BLENDINDICES0;
//	float4	weights : BLENDWEIGHT0;
//};

struct PS_IN
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	float3 binormal : BINORMAL;
	float3 tangent : TANGENT;
	float4 color : COLOR0;
	float3 worldPos : WORLDPOS;
};
//StructuredBuffer<float4x4> AnimFrame : register(t0);


SamplerState Sampler : register(s0);
SamplerComparisonState ShadowCompSampler : register(s1);

#ifndef SKINNED_MESH

PS_IN vsIn(VS_IN input)
{
	PS_IN output = (PS_IN) 0;
	
	float4 worldPos = mul(float4(input.pos, 1.0f), opaqueCosntBuffer.world);
	output.pos = mul(worldPos, coreConstants.currentMatrices.viewProjection);
	//output.pos = output.pos/ output.pos.w;
	output.worldPos = worldPos.xyz/worldPos.w;
    
	output.normal	= normalize(mul(float4(input.normal.xyz, 0.0f),   opaqueCosntBuffer.world)).xyz;
	output.binormal	= normalize(mul(float4(input.binormal.xyz, 0.0f), opaqueCosntBuffer.world)).xyz;
	output.tangent	= normalize(mul(float4(input.tangent.xyz, 0.0f),  opaqueCosntBuffer.world)).xyz;

	output.color = input.color;

	output.tex = input.tex.xy;
	
	return output;
}

#else

//PS_IN VSMain(VS_IN_ANIM input)
//{
//	PS_IN output = (PS_IN) 0;
//
//	float3 animPos = 0;
//	float3 animNorm = 0;
//	float3 animBi = 0;
//	float3 animTan = 0;
//
//	[unroll]
//	for (int i = 0; i < 4; ++i)
//	{
//		float boneInd	= input.inds[i];
//		float weight	= input.weights[i];
//
//		float4x4 frameMat = AnimFrame[boneInd];
//
//		animPos += mul(float4(input.pos.xyz, 1.0f), frameMat).xyz * weight;
//		animNorm += mul(float4(input.normal.xyz, 0.0f), frameMat).xyz * weight;
//		animTan += mul(float4(input.tangent.xyz, 0.0f), frameMat).xyz * weight;
//		animBi += mul(float4(input.binormal.xyz, 0.0f), frameMat).xyz * weight;
//	}
//
//	output.worldPos = mul(float4(animPos, 1.0f), ConstData.World);
//	output.pos = mul(float4(animPos, 1.0f), ConstData.WorldViewProj);
//    
//	output.normal = normalize(mul(float4(animNorm, 0.0f), ConstData.World));
//	output.binormal = normalize(mul(float4(animBi, 0.0f), ConstData.World));
//	output.tangent = normalize(mul(float4(animTan, 0.0f), ConstData.World));
//
//	output.color = input.color;
//
//	output.tex = input.tex.xy;
//	
//	return output;
//}

#endif


struct PSOutput
{
	float4 Diffuse		: SV_Target0; // a - Specular
	float4 Normal		: SV_Target1;
	float4 MetRougAo	: SV_Target2;
	float4 Emissive		: SV_Target3; // Emissive Color
	float4 WorldPos		: SV_Target4;
};


Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetallicMap : register(t3);


PSOutput psIn(PS_IN input)
{
	PSOutput ret = (PSOutput) 0;

	float3 T = normalize(input.tangent.xyz);
	float3 B = normalize(input.binormal.xyz);
	float3 N = normalize(input.normal.xyz);
	float3x3 TBN = float3x3(T, B, N);

#ifndef DIFFUSE
	ret.Diffuse.rgb = DiffuseMap.Sample(Sampler, input.tex.xy).rgb;
#else
	ret.Diffuse.rgb = opaqueCosntBuffer.diffuse.rgb;
#endif
	
#ifdef RED
	ret.Diffuse.rgb = float3(1,input.tex.x,input.tex.y);
#endif
	ret.Diffuse.a = 1.0f;
	
	ret.WorldPos = float4(input.worldPos,1);
	ret.Emissive = input.color;
	
	float roughness;

#ifndef ROUGHNESS
	roughness = RoughnessMap.Sample(Sampler, input.tex.xy).r;
#else
	roughness = opaqueCosntBuffer.roughness;
#endif

	float metallic;
#ifndef METALIC
	metallic	= MetallicMap.Sample(Sampler, input.tex.xy).r;
#else
	metallic = opaqueCosntBuffer.metallic;
#endif
	
	float3 normal;

#ifndef NORMALLL
	normal = NormalMap.Sample(Sampler, input.tex.xy).xyz;
#else
	normal = opaqueCosntBuffer.normal;
#endif
	
	float3 unpackedNormal = normalize(normal * 2.0f - 1.0f);

#ifdef INVERT_NORMAL
	unpackedNormal = -unpackedNormal.xyz;
#endif

	unpackedNormal = mul(unpackedNormal, TBN);
	ret.Normal = float4(N,1);
	
	ret.MetRougAo = float4(metallic * opaqueCosntBuffer.MetallicMult, roughness * opaqueCosntBuffer.RoughnessMult, 1.0f, 1.0f);

	return ret;
}
