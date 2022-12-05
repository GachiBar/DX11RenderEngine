#define HLSL
#define GACHI

#include "../DX11RenderEngine/DX11RenderEngine/include/CoreRenderSystem/CoreShaderInclude.h"
#include "../DX11RenderEngine/GachiRenderSystem/source/RendererPasses/PBRPasses/LightRenderPass/LightPassConstBuffer.h"
struct PSInStruct
{
	float4 pos : SV_Position;
	float2 uv  : TEXCOORD;
};


#ifdef SCREEN_QUAD
struct VS_IN
{
	float2 pos : Position;
	float2 uv : TEXCOORD;
};

#else
struct VS_IN
{
	float4 pos : POSITION0;
	float4 color : COLOR0;
};
#endif


struct PS_IN
{
	float4 pos : SV_POSITION;
};


SamplerState Sampler : register(s0);
SamplerState SamplerClamp : register(s1);
SamplerComparisonState ShadowCompSampler : register(s2);


PS_IN vsIn(
#ifdef SCREEN_QUAD
	uint id: SV_VertexID,
	VS_IN input
#else
	VS_IN input
#endif
)
{
#ifdef SCREEN_QUAD
	PS_IN output = (PS_IN)0;
	output.pos = float4(input.pos, 0.0f, 1.0f);
#else
	PS_IN output = (PS_IN) 0;
	output.pos = mul(mul(float4(input.pos.xyz, 1.0f), lightCosntBuffer.world), coreConstants.currentMatrices.viewProjection);
#endif
	
	return output;
}



struct PSOutput
{
	float4 Accumulator : SV_Target0;
	float4 Bloom : SV_Target1;
};


Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D MetRougAoMap : register(t2);
Texture2D EmissiveMap : register(t3);
Texture2D WorldPosMap : register(t4);

TextureCube PreFiltEnvMap : register(t5);
TextureCube ConMap : register(t6);
Texture2D IntegratedMap : register(t7);

Texture2D ShadowMap : register(t4);


struct GBufferData
{
	float4 Diffuse;
	float3 Normal;
	float4 MetRougAo;
	float3 Emissive;
	float3 WorldPos;
};


GBufferData ReadGBuffer(float2 screenPos)
{
	GBufferData buf = (GBufferData) 0;

	buf.Diffuse		= DiffuseMap.Load(float3(screenPos, 0));
	buf.WorldPos	= WorldPosMap.Load(float3(screenPos, 0)).xyz;
	buf.Emissive	= EmissiveMap.Load(float3(screenPos, 0)).xyz;
	buf.Normal		= NormalMap.Load(float3(screenPos, 0)).xyz;
	buf.MetRougAo	= MetRougAoMap.Load(float3(screenPos, 0));

	return buf;
}

float4 CalculateLight(GBufferData buf, float4 ViewerPos)
{
	float4 color = float4(0, 0, 0, 0);
#ifdef AmbientLight
	color = float4(buf.Diffuse.rgb * Light.Params.x * Light.Color.rgb, 1.0f);
#elif DirectionalLight
	float3 viewDir	= normalize(ViewerPos.xyz - buf.WorldPos);
	float3 lightDir = normalize(lightCosntBuffer.Light.Dir.xyz);
	float3 refVec	= normalize(reflect(-lightDir, buf.Normal));

	float NdotL = saturate(dot(buf.Normal, -lightDir));
	if (NdotL <= 0)        
		discard; // discard as no impact
	
	float3 diffuse	= NdotL * buf.Diffuse.rgb;
	float3 spec		= pow(max(0, dot(-viewDir, refVec)), 50) * buf.Diffuse.a;

	color = float4(lightCosntBuffer.Light.Color.rgb * (diffuse + spec) * lightCosntBuffer.Light.Params.x, 1.0f);
#else
	float3 lightRadVec = lightCosntBuffer.Light.Pos.xyz - buf.WorldPos;
	float distanceToLight = length(lightRadVec);

	float3 viewDir = normalize(ViewerPos.xyz - buf.WorldPos);
	float3 lightDir = normalize(lightRadVec);
	float3 refVec = normalize(reflect(buf.Normal, lightDir));

	float NdotL = saturate(dot(buf.Normal, lightDir));
	if (NdotL <= 0)
		discard; // discard as no impact

	float3 diffuse = NdotL * buf.Diffuse.rgb;
	float3 spec = pow(max(0, dot(refVec, viewDir)), 50) * buf.Diffuse.a;

#if PointLight

	float	atten = max(1.0f - distanceToLight/Light.Params.x, 0);
			atten *= atten;

	color = float4(Light.Color.rgb * (diffuse + spec) * atten * Light.Params.z, 1.0f);

#elif SpotLight

	float rho		= dot(-Light.Dir.xyz, lightDir);
	float cosTheta	= cos(Light.Params.y);
	float cosPhi	= cos(Light.Params.z);

	float atten = 0.0f;
	if (rho > cosTheta)		atten = 1.0f;
	else if (rho <= cosPhi) atten = 0.0f;
	else {
		atten = saturate(pow(abs((rho - cosPhi) / (cosTheta - cosPhi)), Light.Params.w));
	}

	//float distAtten = saturate( 1000.0f / (1.1f + 0.5f * distanceToLight + 0.01f * distanceToLight * distanceToLight) );
	float distAtten = max(1.0f - distanceToLight / Light.Params.x, 0);
	distAtten *= distAtten;

	float intensity = Light.Dir.w;

	color = float4(Light.Color.rgb * intensity * (diffuse + spec) * atten * distAtten, 1.0f);

#endif
#endif

	return color;
}


#define PI 3.1415926535

float GGX_PartialGeometry(float cosThetaN, float alpha)
{
	float cosTheta_sqr = saturate(cosThetaN * cosThetaN);
	float tan2 = (1.0f - cosTheta_sqr) / cosTheta_sqr;
	float GP = 2.0f / (1.0f + sqrt(1 + alpha * alpha * tan2));
	return GP;
}


float GGX_Distribution(float cosThetaNH, float alpha)
{
	float alpha2 = alpha * alpha;
	float NH_sqr = saturate(cosThetaNH * cosThetaNH);
	float den = NH_sqr * alpha2 + (1.0 - NH_sqr);
	return alpha2 / (PI * den * den);
}


float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return num / denom;
}


float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}


float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}


float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}


float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float invRoug = 1.0 - roughness;
	return F0 + (max(invRoug.xxx, F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}



PSOutput psIn(PS_IN input)
{
	PSOutput ret = (PSOutput) 0;

	GBufferData buf = ReadGBuffer(input.pos.xy);

	clip(length(buf.Normal) - 0.001f);

#ifdef DirectionalLight
	float3 L = normalize(lightCosntBuffer.Light.Pos.xyz);
#else
	float3 L = normalize(lightCosntBuffer.Light.Pos.xyz - buf.WorldPos);
#endif
	
	float3 N = normalize(buf.Normal);
	float3 V = normalize(coreConstants.viewPosition.xyz - buf.WorldPos);
	float3 R = reflect(-V, N);
	float3 H = normalize(V + L);
	float3 albedo = buf.Diffuse.rgb;
	float metallic = buf.MetRougAo.r;
	float roughness = buf.MetRougAo.g;
	float ao = 1.0f;
	
	float NdotL = max(dot(N, L), 0.0);
	float NdotV = max(dot(N, V), 0.0);
	
	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	F0 = lerp(F0, albedo, metallic);

#ifdef AmbientLight
	/////// Ambient part - Diffuse Irradiance ////////////////////////////////////////////////////////
	float3 Fa = fresnelSchlickRoughness(NdotV, F0, roughness);
	float3 kSa = Fa;
	float3 kDa = float3(1.0f, 1.0f, 1.0f) - kSa;
	kDa *= 1.0 - metallic;
	float3 irradiance = ConMap.Sample(Sampler, N).rgb;
	float3 diffusea = irradiance * albedo;


	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
	const float MAX_REFLECTION_LOD = 4.0;
	float3 prefilteredColor = PreFiltEnvMap.SampleLevel(Sampler, R, roughness * MAX_REFLECTION_LOD).rgb;
	float2 brdf = IntegratedMap.Sample(SamplerClamp, float2(NdotV, roughness)).rg;
	float3 speculara = prefilteredColor * (Fa * brdf.x + brdf.yyy);


	float3 ambient = (kDa * diffusea * Light.Params.x + speculara * Light.Params.y) * ao;

	ret.Accumulator = float4(ambient, 1.0f);
	ret.Bloom = float4(1.0f, 1.0f, 1.0f, 1.0f);
	/////////////////////////////////////////////////////////////////////////////
#else
	
#ifdef PointLight
	// calculate per-light radiance
	float distance = length(Light.Pos.xyz - buf.WorldPos);
	float attenuation = max(1.0f - distance / Light.Params.x, 0);
	attenuation *= attenuation;
	float3 radiance = attenuation * Light.Params.z;
#else
	float3 radiance = lightCosntBuffer.Light.Params.xxx;
#endif

	// cook-torrance brdf
	float	NDF	= DistributionGGX(N, H, roughness);
	float	G	= GeometrySmith(N, V, L, roughness);
	float3	F	= fresnelSchlick(max(dot(H, V), 0.0), F0);

	float3 kS = F;
	float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
	kD *= 1.0 - metallic;

	float3 numerator = NDF * G * F;
	float denominator = 4.0 * NdotV * NdotL;
	float3 specular = numerator / max(denominator, 0.001);

	// reflectance equation
	float3 Lo = float3(0, 0, 0);
	Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  
	float3 color = Lo;

	ret.Accumulator = float4(color, 1.0f);

	float4 bloom = float4(1.0f, 1.0f, 1.0f, 1.0f);
	ret.Bloom = bloom;

#endif

	return ret;
}
