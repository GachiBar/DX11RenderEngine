#define HLSL
#define GACHI

#include "../DX11RenderEngine/DX11RenderEngine/include/CoreRenderSystem/CoreShaderInclude.h"
#include "../DX11RenderEngine/GachiRenderSystem/source/RendererPasses/PBRPasses/PostProcessRenderPass/PostProcessRenderPassConstBuffer.h"

struct PS_IN
{
	float4 pos : SV_POSITION;
};


PS_IN vsIn(uint id : SV_VertexID)
{
	PS_IN output = (PS_IN) 0;
	float2 inds = float2(id & 1, (id & 2) >> 1);
	output.pos = float4(inds * float2(2, -2) + float2(-1, 1), 0, 1);

	return output;
}



Texture2D AccumMap : register(t0);
Texture2D BloomMap : register(t1);


static const float MIDDLE_GRAY = 0.72f;
static const float LUM_WHITE = 1.5f;



float4 psIn(PS_IN input) : SV_Target0
{
	float4 final = float4(0.25f, 0.25f, 0.25f, 1.0f);
	
	float3 hdr = AccumMap.Load(int3(input.pos.xy, 0)).rgb;

#ifdef RED
if (hdr != float3(0,0,0}))
	return float4(1,0,0, 1);
else
	return float4(0,0,0, 1);

#endif

//#ifdef CopyColor
//	final.rgb = hdr;
//#elif Simple
//	
//	final.rgb = hdr.rgb / (hdr + 1.0f);
//	
//#elif WithLum
//	
//	final.rgb = float3(1.0f, 1.0f, 1.0f) - exp(-hdr * postProcessCosntBuffer.lum);
//
//#elif WithLumEx
	
	//float fLum = lum[0]*g_param.x;
    float3 vBloom = float3(0,0,0); // bloom.Sample( LinearSampler, Input.Tex );
    // Tone mapping
    final.rgb = hdr.rgb;
	final.rgb *= MIDDLE_GRAY / (1 + 0.001f);
    final.rgb *= (1.0f + final.rgb/LUM_WHITE);
    final.rgb /= (1.0f + final.rgb);
    final.rgb += 0.6f * vBloom; // Adding bloom
	
//#endif
	
	final.a = 1.0f;
	
	final.rgb = pow(abs(final.rgb), 1.0f / 2.2f);
	
	return final;
}
