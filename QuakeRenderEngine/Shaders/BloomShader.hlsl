#define HLSL
#include "ref_dx11rg\DX11RenderEngine\DX11RenderEngine\source\CoreRenderSystem\CoreShaderInclude.h"
#include "ref_dx11rg\DX11RenderEngine\DX11RenderEngine\source\CoreRenderSystem\Renderers\BloomRenderer\BloomConstBuffer.h"

struct VSIn {
	float2 pos : Position;
	float2 uv  : TEXCOORD;
};

struct PSIn {
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD;
};

PSIn vsIn(VSIn input) {
	PSIn output = (PSIn)0;

	output.pos = float4(input.pos, 0.0f, 1.0f);
	output.uv = input.uv;
	return output;
}

Texture2D bloomColor : register(t0);

SamplerState basicSampler : register(s0);


float gauss_weight(int sampleDist, float sigma)
{
	float g = 1.0f / sqrt(2.0f * 3.14159 * sigma * sigma);
	return (g * exp(-(sampleDist * sampleDist) / (2 * sigma * sigma)));
}

float4 gauss(in PSIn input, in float2 texDirr, in float sigma)
{
	float4 color = 0;

	float w,h;
	bloomColor.GetDimensions(w,h);

	for (int i = 1; i <= BloomCosntants.radius; i++)
	{
		float weight = gauss_weight(i, sigma);

		// compute tap tc
		float2 tc1 = input.uv;
		float2 tc2 = input.uv;
		tc1 += (i / float2(w,h)) * texDirr;
		tc2 -= (i / float2(w,h)) * texDirr;

		color += (bloomColor.Sample(basicSampler, tc1)  + bloomColor.Sample(basicSampler, tc2))* weight;
	}
	color += bloomColor.Sample(basicSampler, input.uv) * gauss_weight(0, sigma);;
	return color;
}

#ifdef HORISONTAL
float4 psIn(PSIn input) : SV_Target
{
	return gauss(input, float2(0, 1), BloomCosntants.sigma);
}
#endif

#ifdef VERTICAL
float4 psIn(PSIn input) : SV_Target
{
	return gauss(input, float2(1, 0), BloomCosntants.sigma);
}
#endif

#ifdef THRESHOLD

float CalcLuminance(float3 color)
{
	return max(dot(color, float3(0.299f, 0.587f, 0.114f)), 0.0001f);
}
float4 psIn(PSIn input) : SV_Target
{
	float4 color =  bloomColor.Sample(basicSampler, input.uv);
	float lum =  CalcLuminance(color);
	lum = saturate(lum*4-3);
	if (lum > BloomCosntants.threshold)
		return color * lum;
	else
		return 0;
}
#endif

#ifdef END
float4 psIn(PSIn input) : SV_Target
{
	return bloomColor.Sample(basicSampler, input.uv) * BloomCosntants.intensity;
}
#endif