#define HLSL
#include "ref_dx11rg\DX11RenderEngine\DX11RenderEngine\include\CoreRenderSystem/CoreShaderInclude.h"
#include "ref_dx11rg\DX11RenderEngine\QuakeRenderEngine\source\RendererPasses\SSAORenderPass\SSAORendererConstBuffer.h"

//#define BLUR

static const int gSampleCount = 14;


// Determines how much the sample point q occludes the point p as a function
// of distZ.
float OcclusionFunction(float distZ)
{
	//
	// If depth(q) is "behind" depth(p), then q cannot occlude p.  Moreover, if 
	// depth(q) and depth(p) are sufficiently close, then we also assume q cannot
	// occlude p because q needs to be in front of p by Epsilon to occlude p.
	//
	// We use the following function to determine the occlusion.  
	// 
	//
	//       1.0     -------------\
	//               |           |  \
	//               |           |    \
	//               |           |      \ 
	//               |           |        \
	//               |           |          \
	//               |           |            \
	//  ------|------|-----------|-------------|---------|--> zv
	//        0     Eps          z0            z1        
	//
	
	float occlusion = 0.0f;
	if(distZ > SSAOData.gSurfaceEpsilon)
	{
		float fadeLength = SSAOData.gOcclusionFadeEnd - SSAOData.gOcclusionFadeStart;
		
		// Linearly decrease occlusion from 1 to 0 as distZ goes 
		// from gOcclusionFadeStart to gOcclusionFadeEnd.	
		occlusion = saturate( (SSAOData.gOcclusionFadeEnd-distZ)/fadeLength );
	}
	
	return occlusion;	
}

#ifdef OCCLUSION
// Nonnumeric values cannot be added to a cbuffer.
Texture2D gNormalMap    : register(t0);
Texture2D gDepthMap     : register(t1);
Texture2D gRandomVecMap : register(t2);

RWTexture2D<float> Result : register( u0 );

SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearClamp : register(s1);
SamplerState gsamDepthMap : register(s2);
SamplerState gsamLinearWrap : register(s3);




[numthreads( SSAO_NUM_THREADS_X, SSAO_NUM_THREADS_Y, 1 )]
void main( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID ) {
	// p -- the point we are computing the ambient occlusion for.
	// n -- normal vector at p.
	// q -- a random offset from p.
	// r -- a potential occluder that might occlude p.

	//int2 GroupUL = (Gid.xy << 3) - 4;                // Upper-left pixel coordinate of group read location
	//int2 ThreadUL = (GTid.xy << 1) + GroupUL;        // Upper-left pixel coordinate of quad that this thread will read

	float w,h;
	gDepthMap.GetDimensions(w,h);
	
	int2 TexC = DTid.xy;
	float2 uv = float2(DTid.x, DTid.y) /(float2(w,h)) ;

	

	// Get viewspace normal and z-coord of this pixel.  
    float3 n = normalize(gNormalMap.SampleLevel(gsamPointClamp, uv, 0.0f).xyz) ;
	n = normalize(mul(float4(n, 0), coreConstants.currentMatrices.view)).xyz;
    float pz = gDepthMap.SampleLevel(gsamDepthMap, uv, 0.0f).r;;
	
	
	float4 H = float4(uv.x * 2 - 1, (1 - uv.y) * 2 - 1, pz, 1);
	// Transform by the view-projection inverse.
	float4 D = mul(H, coreConstants.currentMatrices.inverseProjection);
    //pz = NdcDepthToViewDepth(pz);
	

	//
	// Reconstruct full view space position (x,y,z).
	// Find t such that p = t*PosV.
	// p.z = t*PosV.z
	// t = p.z / PosV.z
	//
	float3 p = D.xyz/D.w;
	// Extract random vector and map from [0,1] --> [-1, +1].
	float3 randVec = 2.0f*gRandomVecMap.SampleLevel(gsamLinearWrap, 4.0f*uv*1.0, 0.0f).rgb - 1.0f;

	float occlusionSum = 0.0f;
	
	// Sample neighboring points about p in the hemisphere oriented by n.
	
	for(int i = 0; i < gSampleCount; ++i)
	{
		// Are offset vectors are fixed and uniformly distributed (so that our offset vectors
		// do not clump in the same direction).  If we reflect them about a random vector
		// then we get a random uniform distribution of offset vectors.
		float3 offset = reflect(SSAOData.gOffsetVectors[i].xyz, randVec);
	
		// Flip offset vector if it is behind the plane defined by (p, n).
		float flip = sign( dot(offset, n) );
		
		// Sample a point near p within the occlusion radius.
		float3 q = p + flip * SSAOData.gOcclusionRadius * offset;
		
		// Project q and generate projective tex-coords.  
		//float4 projQ = mul(float4(q, 1.0f), mainConstants.viewProjection);
		float4 projQ = mul(float4(q, 1.0f), SSAOData.gProjTex);
		projQ /= projQ.w;

		// Find the nearest depth value along the ray from the eye to q (this is not
		// the depth of q, as q is just an arbitrary point near p and might
		// occupy empty space).  To find the nearest depth we look it up in the depthmap.

		float rz = gDepthMap.SampleLevel(gsamDepthMap, projQ.xy, 0.0f).r;
       // rz = NdcDepthToViewDepth(rz);

		// Reconstruct full view space position r = (rx,ry,rz).  We know r
		// lies on the ray of q, so there exists a t such that r = t*q.
		// r.z = t*q.z ==> t = r.z / q.z
		H = float4(projQ.x * 2 - 1, (1 - projQ.y) * 2 - 1, rz, 1);
		D = mul(H, coreConstants.currentMatrices.inverseProjection);
		float3 r = D.xyz/D.w;
		
		//
		// Test whether r occludes p.
		//   * The product dot(n, normalize(r - p)) measures how much in front
		//     of the plane(p,n) the occluder point r is.  The more in front it is, the
		//     more occlusion weight we give it.  This also prevents self shadowing where 
		//     a point r on an angled plane (p,n) could give a false occlusion since they
		//     have different depth values with respect to the eye.
		//   * The weight of the occlusion is scaled based on how far the occluder is from
		//     the point we are computing the occlusion of.  If the occluder r is far away
		//     from p, then it does not occlude it.
		// 
		
		float distZ = p.z - r.z;
		float dp = max(dot(n, normalize(r - p)), 0.0f);

        float occlusion = dp*OcclusionFunction(distZ);

		occlusionSum += occlusion;
	}
	
	occlusionSum /= gSampleCount;
	
	float access = 1.0f - occlusionSum;

	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
	Result[TexC] = saturate(pow(access, 6.0f));
}

#endif
//#define BLUR
#ifdef BLUR

static const int gMaxBlurRadius = 5;


Texture2D<float> InputBuf        : register(t0);
RWTexture2D<float> Result : register(u0);


// The guassian blur weights (derived from Pascal's triangle)

static  float Weights[5];

float BlurPixels( float a, float b, float c, float d, float e, float f, float g, float h, float i )
{
    return Weights[0]*e + Weights[1]*(d+f) + Weights[2]*(c+g) + Weights[3]*(b+h) + Weights[4]*(a+i);
}

// 16x16 pixels with an 8x8 center that we will be blurring writing out.  Each uint is two color channels packed together
groupshared uint CacheR[128];
groupshared uint CacheG[128];
groupshared uint CacheB[128];

void Store2Pixels( uint index, float pixel1, float pixel2 )
{
    CacheR[index] = f32tof16(pixel1.r) | f32tof16(pixel2.r) << 16;
    CacheG[index] = f32tof16(0) | f32tof16(0) << 16;
    CacheB[index] = f32tof16(0) | f32tof16(0) << 16;
}

void Load2Pixels( uint index, out float pixel1, out float pixel2 )
{
    uint rr = CacheR[index];
    uint gg = CacheG[index];
    uint bb = CacheB[index];
    pixel1 = float( f16tof32(rr      ) );
    pixel2 = float( f16tof32(rr >> 16));
}

void Store1Pixel( uint index, float3 pixel )
{
    CacheR[index] = asuint(pixel.r);
    CacheG[index] = asuint(0);
    CacheB[index] = asuint(0);
}

void Load1Pixel( uint index, out float pixel )
{
    pixel = (asfloat( uint3(CacheR[index], CacheG[index], CacheB[index]) )).r;
}

// Blur two pixels horizontally.  This reduces LDS reads and pixel unpacking.
void BlurHorizontally( uint outIndex, uint leftMostIndex )
{
    float s0, s1, s2, s3, s4, s5, s6, s7, s8, s9;
    Load2Pixels( leftMostIndex + 0, s0, s1 );
    Load2Pixels( leftMostIndex + 1, s2, s3 );
    Load2Pixels( leftMostIndex + 2, s4, s5 );
    Load2Pixels( leftMostIndex + 3, s6, s7 );
    Load2Pixels( leftMostIndex + 4, s8, s9 );
    
    Store1Pixel(outIndex  , BlurPixels(s0, s1, s2, s3, s4, s5, s6, s7, s8));
    Store1Pixel(outIndex+1, BlurPixels(s1, s2, s3, s4, s5, s6, s7, s8, s9));
}

void BlurVertically( uint2 pixelCoord, uint topMostIndex )
{
    float s0, s1, s2, s3, s4, s5, s6, s7, s8;
    Load1Pixel( topMostIndex   , s0 );
    Load1Pixel( topMostIndex+ 8, s1 );
    Load1Pixel( topMostIndex+16, s2 );
    Load1Pixel( topMostIndex+24, s3 );
    Load1Pixel( topMostIndex+32, s4 );
    Load1Pixel( topMostIndex+40, s5 );
    Load1Pixel( topMostIndex+48, s6 );
    Load1Pixel( topMostIndex+56, s7 );
    Load1Pixel( topMostIndex+64, s8 );

    Result[pixelCoord] = saturate(BlurPixels(s0, s1, s2, s3, s4, s5, s6, s7, s8))*SSAOData.intensity;
}

float gauss_weight(int sampleDist, float sigma)
{
	float g = 1.0f / sqrt(2.0f * 3.14159 * sigma * sigma);
	return (g * exp(-(sampleDist * sampleDist) / (2 * sigma * sigma)));
}


[numthreads( 8, 8, 1 )]
void blur( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{

	Weights[0]=gauss_weight(0, SSAOData.sigma);
	Weights[1]=gauss_weight(1, SSAOData.sigma);
	Weights[2]=gauss_weight(2, SSAOData.sigma);
	Weights[3]=gauss_weight(3, SSAOData.sigma);
	Weights[4]=gauss_weight(4, SSAOData.sigma);
	
    //
    // Load 4 pixels per thread into LDS
    //
    int2 GroupUL = (Gid.xy << 3) - 4;                // Upper-left pixel coordinate of group read location
    int2 ThreadUL = (GTid.xy << 1) + GroupUL;        // Upper-left pixel coordinate of quad that this thread will read

    //
    // Store 4 unblurred pixels in LDS
    //
    int destIdx = GTid.x + (GTid.y << 4);
    Store2Pixels(destIdx+0, InputBuf[ThreadUL + uint2(0, 0)], InputBuf[ThreadUL + uint2(1, 0)]);
    Store2Pixels(destIdx+8, InputBuf[ThreadUL + uint2(0, 1)], InputBuf[ThreadUL + uint2(1, 1)]);

    GroupMemoryBarrierWithGroupSync();

    //
    // Horizontally blur the pixels in Cache
    //
    uint row = GTid.y << 4;
    BlurHorizontally(row + (GTid.x << 1), row + GTid.x + (GTid.x & 4));

    GroupMemoryBarrierWithGroupSync();

    //
    // Vertically blur the pixels and write the result to memory
    //
    BlurVertically(DTid.xy, (GTid.y << 3) + GTid.x);
}

#endif


#ifdef COPY

Texture2D<float> InputBuf        : register(t0);
RWTexture2D<float> Result : register(u0);

[numthreads( SSAO_NUM_THREADS_X, SSAO_NUM_THREADS_Y, 1 )]
void copy( uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	//
	// Load 4 pixels per thread into LDS
	//
	int2 GroupUL = (Gid.xy << 3) - 4;                // Upper-left pixel coordinate of group read location
	int2 ThreadUL = (GTid.xy << 1) + GroupUL;        // Upper-left pixel coordinate of quad that this thread will read
	
	Result[DTid.xy] = InputBuf[DTid.xy].x;
}


#endif