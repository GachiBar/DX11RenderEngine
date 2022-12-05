#define HLSL
#include "ref_dx11rg\DX11RenderEngine\DX11RenderEngine\include\CoreRenderSystem/CoreShaderInclude.h"
#include "ref_dx11rg\DX11RenderEngine\QuakeRenderEngine\source\RendererPasses/PPRenderPass/PPConstBuffer.h"

struct VSIn
{
    float2 pos : Position;
    float2 uv : TEXCOORD;
};

struct PSIn
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

PSIn vsIn(VSIn input)
{
    PSIn output = (PSIn)0;

    output.pos = float4(input.pos, 0.0f, 1.0f);
    output.uv = input.uv;
    return output;
}


Texture2D diffuseColor : register(t0);

Texture2D bloomMask : register(t1);
Texture2D lightmap : register(t2);;
Texture2D alphaSurfaces : register(t3);
Texture2D occlusion : register(t4);


SamplerState blureSampler : register(s0);
SamplerState pointSampler : register(s1);

float4 psIn(PSIn input) : SV_Target
{
    float4 x = 0;
    float2 texCoord = input.uv;

#ifdef SINGLETEXTURE
    float4 output = diffuseColor.Sample(blureSampler, texCoord);
    
    //if (output != abs(output))
    //{
    //    output += float4(1,1,1,1);
    //    output/=2;
    //}
    return float4(output.xyz, 1); 
#endif
    


#ifdef ALPHA
	float4 alphaColor = alphaSurfaces.Sample(blureSampler, texCoord);
	//float depth = depthTex.Sample(blureSampler, texCoord);
    alphaColor.w = 0.3;
	return alphaColor;
#endif

    float4 color = diffuseColor.Sample(blureSampler, texCoord);
    float4 light = lightmap.Sample(blureSampler, texCoord);
    light = light * occlusion.Sample(pointSampler, texCoord).xxxx;
    light.rgb += ppCosntBuffer.lightAdd.xxx;
    float4 bloom = bloomMask.Sample(blureSampler, texCoord);
    
    
    return color*light + bloom;

}
