#include "pch.h"
#include "SSAORenderPass.h"

#include <d3d11.h>

#include "imgui/imgui.h"
#include "../TAARenderPass/halton.h"
#include "ResourceManagers/States/BlendStates.h"
#include "ResourceManagers/States/DSStates.h"
#include "ResourceManagers/States/RasterizerStates.h"
#include "ResourceManagers/States/Samplers.h"

using namespace Renderer;

SSAORenderPass::SSAORenderPassProvider::SSAORenderPassProvider() {}

const char* SSAORenderPass::SSAORenderPassProvider::GetShaderName()
{
	return "SSAO";
}

const Renderer::ShaderDefines SSAORendererDefines[] = {
	Renderer::ShaderDefines("RED"),
	Renderer::ShaderDefines("BLUR"),
	Renderer::ShaderDefines("OCCLUSION"),
	Renderer::ShaderDefines("COPY"),
};

Renderer::PipelineFactoryDescription SSAORenderPass::SSAORenderPassProvider::GetFactoryDescription()
{
	return {
		SSAORendererDefines,
		std::size(SSAORendererDefines),
		UseComputeShader
	};
}

void SSAORenderPass::SSAORenderPassProvider::PatchPipelineState(Renderer::Pipeline* refToPS, size_t definesFlags) {

	refToPS->bs = &BlendStates::NoAlpha;
	refToPS->dss = &DepthStencilStates::NoDSS;
	refToPS->rs = &RasterizerStates::All;

}
const D3D11_INPUT_ELEMENT_DESC DefaultInputElements[] =
{
		{"Position",  0, DXGI_FORMAT_R32G32_FLOAT,  0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
};


Renderer::InputLayoutDescription SSAORenderPass::SSAORenderPassProvider::GetInputLayoutDescription(size_t definesFlags) {
	return Renderer::InputLayoutDescription{ (void*)DefaultInputElements, std::size(DefaultInputElements) };
}


float RandF()
{
	return (float)(rand()) / (float)RAND_MAX;
}

// Returns random float in [a, b).
float RandF(float a, float b)
{
	return a + RandF()*(b-a);
}

SSAORenderPass::SSAORenderPass(BaseRenderSystem& System):
	BaseRenderPass({"SSAOShader.hlsl", System}),
	QuadHelper(System.pRenderer)
{
	
	uint32_t width, height;
	renderDevice->GetBackbufferSize(width, height);
	
	randVect = renderDevice->CreateTexture2D(SURFACEFORMAT_VECTOR4,256, 256,1, 0);

	//double* randData = halton_sequence(10,256*256*10, 3);
	std::vector<float4> initData(256*256);
	
	for(int i = 0; i < 256; ++i)
	{
		for(int j = 0; j < 256; ++j)
		{
			initData[j * 256 + i] = float4(RandF(), RandF(), RandF(), 0);
		}
	}
	//delete[] randData;
	renderDevice->SetTextureData2D(randVect,0,0,256,256,0,initData.data(),256*256*16);

	float4 mOffsets[14];
	
	mOffsets[0] = float4(+1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[1] = float4(-1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[2] = float4(-1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[3] = float4(+1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[4] = float4(+1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[5] = float4(-1.0f, -1.0f, +1.0f, 0.0f);

	mOffsets[6] = float4(-1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[7] = float4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	mOffsets[8] =  float4(-1.0f, 0.0f, 0.0f, 0.0f);
	mOffsets[9] =  float4(+1.0f, 0.0f, 0.0f, 0.0f);

	mOffsets[10] = float4(0.0f, -1.0f, 0.0f, 0.0f);
	mOffsets[11] = float4(0.0f, +1.0f, 0.0f, 0.0f);

	mOffsets[12] = float4(0.0f, 0.0f, -1.0f, 0.0f);
	mOffsets[13] = float4(0.0f, 0.0f, +1.0f, 0.0f);

	for(int i = 0; i < 14; ++i)
	{
		// Create random lengths in [0.25, 1.0].
		float s = RandF(0.25f, 1.0f);
		mOffsets[i].Normalize();
		
		float4 v = s * mOffsets[i];

		localBuffer.gOffsetVectors[i] = v;
	}
	
	/*
	localBuffer.depthThreshold  = 0.0001;
	localBuffer.DilationMode = 1;
	localBuffer.ReprojectionMode = 1;
	localBuffer.NeighborhoodClampMode = 1;
	*/
	localBuffer.sigma = 3;
	localBuffer.intensity = 1;
	localBuffer.kernel = 1;
	//localBuffer.radius = 5;
	localBuffer.gInvRenderTargetSize = float2(1.f/width,1.f/height);
	
	localBuffer.gOcclusionRadius    = 1.4;
	localBuffer.gOcclusionFadeStart = 0.3;
	localBuffer.gOcclusionFadeEnd   = 0.7;
	localBuffer.gSurfaceEpsilon     = 0.05;
	
	constBuffer = renderDevice->CreateConstBuffer(sizeof(localBuffer));

	
	baseRendererParams.renderSystem.texturesManger->CreatePrivateRenderTarget(
		{"SSAObuff",SURFACEFORMAT_SINGLE,true,true, false, width, height}, buff, buffRT);
}

void SSAORenderPass::Init(const char* dirr) {

	BaseRenderPass::Init(dirr, new SSAORenderPassProvider());
	
}

void SSAORenderPass::Render() {
	QuadHelper.Render();

	matrix T(
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f);
	localBuffer.gProjTex =
		(baseRendererParams.renderSystem.viewConstants.currentMatrices.projection.Transpose()*T).Transpose();
	
	uint32_t width, height;
	renderDevice->GetBackbufferSize(width, height);

	

	size_t flags = SSAOZERO;
	
	renderDevice->SetRenderTargets(nullptr, 0, nullptr);
	
	renderDevice->SetConstBuffer(constBuffer, &localBuffer);
	renderDevice->VerifyConstBuffer(constBuffer, SSAOData.slot);

	
	
	renderDevice->VerifyPixelSampler(0, Samplers::pointClamp);
	renderDevice->VerifyPixelSampler(1, Samplers::linearClamp);
	renderDevice->VerifyPixelSampler(2, Samplers::linearClamp);
	renderDevice->VerifyPixelSampler(3, Samplers::linearWrap);
	
	
	renderDevice->VerifyPixelTexture(0, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("normalsField"))->texture);
	renderDevice->VerifyPixelTexture(1, baseRendererParams.renderSystem.texturesManger->depthBuffer->texture);
	renderDevice->VerifyPixelTexture(2, randVect);

	renderDevice->VerifyUATexture(0,  baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("oclusionField"))->texture);
	
	renderDevice->ApplyPipelineState(factory->GetComputeState(flags|SSAOOCCLUSION, "main"));
	//renderDevice->DrawIndexedPrimitives(PrimitiveType::PRIMITIVETYPE_TRIANGLELIST, 0, 0, 0, 0, 2);
	renderDevice->Dispatch((width + SSAO_NUM_THREADS_X -1) / (SSAO_NUM_THREADS_X),
			(height + SSAO_NUM_THREADS_Y -1) / (SSAO_NUM_THREADS_Y));


	
	
	renderDevice->VerifyUATexture(0, nullptr);
	renderDevice->VerifyPixelTexture(0, nullptr);

	//for (int i = 0; i < localBuffer.kernel; i++)
	//{

	renderDevice->VerifyUATexture(0,  baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("oclusionField"))->texture);
	
	//renderDevice->VerifyPixelTexture(0,buff );
	renderDevice->ApplyPipelineState(factory->GetComputeState(SSAOBLUR, "blur"));

	auto*  p1=   baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("oclusionField"))->texture;
	auto*  p2=  buff;
	
	for (int i =0; i<localBuffer.kernel; i++)
	{
		renderDevice->VerifyUATexture(0, p2);
		renderDevice->VerifyPixelTexture(0, p1);
		renderDevice->ApplyPipelineState(factory->GetComputeState(SSAOBLUR, "blur"));
		renderDevice->Dispatch((width + SSAOBLUR_NUM_THREADS -1) / (SSAOBLUR_NUM_THREADS),
							(height + SSAOBLUR_NUM_THREADS -1) / (SSAOBLUR_NUM_THREADS) );
		renderDevice->VerifyUATexture(0, nullptr);
		renderDevice->VerifyPixelTexture(0, nullptr);

		std::swap(p1,p2);
	}

	if (p2 == buff)
	{
		renderDevice->VerifyPixelTexture(0, buff);
		renderDevice->VerifyUATexture(0, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("oclusionField"))->texture);

		renderDevice->ApplyPipelineState(factory->GetComputeState(SSAOCOPY, "copy"));
		//renderDevice->DrawIndexedPrimitives(PrimitiveType::PRIMITIVETYPE_TRIANGLELIST, 0, 0, 0, 0, 2);
		renderDevice->Dispatch((width + SSAO_NUM_THREADS_X -1) / (SSAO_NUM_THREADS_X),
				(height + SSAO_NUM_THREADS_Y -1) / (SSAO_NUM_THREADS_Y));
		
		renderDevice->VerifyUATexture(0, nullptr);
		renderDevice->VerifyPixelTexture(0, nullptr);

	}
	
	renderDevice->VerifyUATexture(0, nullptr);
	renderDevice->VerifyPixelTexture(0, nullptr);
		
		
	//}
		
	RenderIMGUI();
	
}

void SSAORenderPass::PreRender()
{

	RenderTargetBinding* targets[] = {
		baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("oclusionField")),
	};

	renderDevice->SetRenderTargets(targets, std::size(targets), nullptr);
	renderDevice->Clear(CLEAROPTIONS_TARGET, {1,0,0,0}, 0, 0);
}

void SSAORenderPass::PostRender()
{
	
}


void SSAORenderPass::RenderIMGUI()
{	
	ImGui::Begin("SSAO settings.");                          // Create a window called "Hello, world!" and append into it.

	ImGui::SliderFloat("gOcclusionRadius  ",&localBuffer.gOcclusionRadius   ,0,3);
	ImGui::SliderFloat("gOcclusionFadeStar",&localBuffer.gOcclusionFadeStart,0,3);
	ImGui::SliderFloat("gOcclusionFadeEnd ",&localBuffer.gOcclusionFadeEnd  ,0,3);
	ImGui::SliderFloat("gSurfaceEpsilon   ",&localBuffer.gSurfaceEpsilon    ,0,3);
	
	ImGui::SliderFloat("intencity   ",&localBuffer.intensity    ,0,10);
	ImGui::SliderFloat("sigma   ",&localBuffer.sigma   ,0.01,5);
	ImGui::SliderInt("kernel   ",&localBuffer.kernel   ,1,10);
	//ImGui::SliderInt("radius   ",&localBuffer.radius   ,0,SSAOBLUR_NUM_THREADS);
	ImGui::End();
}


SSAORenderPass::~SSAORenderPass()
{
	renderDevice->AddDisposeConstBuffer(constBuffer);
	delete factory;
}
