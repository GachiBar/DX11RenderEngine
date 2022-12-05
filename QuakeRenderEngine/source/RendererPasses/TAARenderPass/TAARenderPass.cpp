#include "TAARenderPass.h"

#include "halton.h"
#include "TAARenderPassProvider.h"
#include "imgui/imgui.h"
#include "ResourceManagers/States/Samplers.h"

using namespace Renderer;

TAARenderPass::TAARenderPass(BaseRenderSystem& System)
        : BaseRenderPass({"TAAShader.hlsl", System}), QuadHelper(System.pRenderer)
{
	Settings.numSamples = 10;
	Settings.taaStrength = 1;
	
	uint32_t width, height;
	renderDevice->GetMainViewportSize(width, height);
	
   //System.texturesManger->CreatePublicRenderTarget(
   //	{"preAAcolor", Renderer::SURFACEFORMAT_VECTOR4, true, false, false, width, height}
   //	);
}

void TAARenderPass::Init(const char* dirr)
{
    BaseRenderPass::Init(dirr, new TAARenderPassProvider());
    
	uint32_t width, height;
	renderDevice->GetMainViewportSize(width, height);
    
    TAAHistory = renderDevice->CreateUATexture2D(Renderer::SURFACEFORMAT_VECTOR4,width, height,1);
	constBuffer = renderDevice->CreateConstBuffer(sizeof(localBuffer));
}
void TAARenderPass::UpdateHaltonSequence()
{
    if (localBuffer.numSamples == 0) {
        localBuffer.numSamples = 10;
    }

    if (HaltonSequence.size() < localBuffer.numSamples)
    {
    	uint32_t width, height;
    	renderDevice->GetMainViewportSize(width, height);
		
        HaltonSequence.resize(localBuffer.numSamples);
        auto left = halton_sequence(20, 20 + localBuffer.numSamples-1, 2);
        //auto right = halton_sequence(20, 20 + localBuffer.numSamples-1, 3);

        for (int i = 0; i < localBuffer.numSamples*2; i+=2)
        {
            HaltonSequence[i/2] = float2(left[i], left[i+1]);
            HaltonSequence[i/2] = (float2(left[i], left[i+1])-float2(0.5,0.5)) / float2(width,height);
        }
        delete[] left;
        //delete[] right;
    }
    HaltonIndex = (HaltonIndex + 1) %localBuffer.numSamples;
}

void TAARenderPass::PreRender()
{	
	uint32_t width, height;
	renderDevice->GetMainViewportSize(width, height);
	
    UpdateHaltonSequence();
    
    localBuffer.Jitter = HaltonSequence[HaltonIndex];
    localBuffer.Resolution = float4(width, height, 1.f/width, 1.f/height);
    localBuffer.FrameNumber = HaltonIndex;
	
    localBuffer.numSamples = Settings.numSamples;
    baseRendererParams.renderSystem.viewConstants.taaBuffer.taaStrength = Settings.taaStrength;
    baseRendererParams.renderSystem.viewConstants.taaBuffer.taaPixelShift = HaltonSequence[HaltonIndex];

	RenderTargetBinding* targets[] = {
		baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("preAAcolor"))
	};

	renderDevice->SetRenderTargets(targets, std::size(targets), nullptr);
	renderDevice->Clear(CLEAROPTIONS_TARGET, {}, 0, 0);
}

void TAARenderPass::Resize()
{
	uint32_t width, height;
	renderDevice->GetMainViewportSize(width, height);
	
    renderDevice->AddDisposeTexture(TAAHistory);
	TAAHistory = renderDevice->CreateUATexture2D(Renderer::SURFACEFORMAT_VECTOR4,width, height,1);
}

void TAARenderPass::Render()
{
	uint32_t width, height;
	renderDevice->GetMainViewportSize(width, height);
	renderDevice->ClearState();
	baseRendererParams.renderSystem.Present();
	

	size_t flags = TAAZERO;
	localBuffer.DebugFlags = 0;
	if (Settings.markNoHistoryPixels)
		localBuffer.DebugFlags |= MarkNoHistoryPixels;
	
	if (Settings.allowBicubicFilter)
		localBuffer.DebugFlags |= AllowBicubicFilter;
	
	if (Settings.allowDepthThreshold)
		localBuffer.DebugFlags |= AllowDepthThreshold;
	
	if (Settings.allowNeighbourhoodSampling)
		localBuffer.DebugFlags |= AllowNeighbourhoodSampling;
	
	if (Settings.allowVarianceClipping)
		localBuffer.DebugFlags |= AllowVarianceClipping;
	
	if (Settings.allowLongestVelocityVector)
		localBuffer.DebugFlags |= AllowLongestVelocityVector;
	
	if (Settings.allowYCoCg)
		localBuffer.DebugFlags |= AllowYCoCg;
	
	
	renderDevice->SetConstBuffer(constBuffer, &localBuffer);
	renderDevice->VerifyConstBuffer(constBuffer, taaCosntBuffer.slot);
	
	renderDevice->VerifyPixelSampler(0, Samplers::pointClamp);
	renderDevice->VerifyPixelSampler(1, Samplers::linearClamp);
	
	renderDevice->SetRenderTargets(nullptr, 0, nullptr);
	
	renderDevice->VerifyPixelTexture(0, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("velocityField"		 ))->texture);
	//renderDevice->VerifyPixelTexture(1, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("diffuseColor"			 ))->texture);
	renderDevice->VerifyPixelTexture(1, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("preAAcolor"			 ))->texture);
	renderDevice->VerifyPixelTexture(2, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("pastColor"			 ))->texture);
	renderDevice->VerifyPixelTexture(3, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("pastDepth"		     ))->texture);
	renderDevice->VerifyPixelTexture(4, baseRendererParams.renderSystem.texturesManger->depthBuffer->texture					     );
	renderDevice->VerifyUATexture(0,    TAAHistory);
	
	renderDevice->ApplyPipelineState(factory->GetComputeState(flags, "main"));
	renderDevice->Dispatch((width + TAA_NUM_THREADS_X -1) / (TAA_NUM_THREADS_X),
		(height + TAA_NUM_THREADS_Y -1) / (TAA_NUM_THREADS_Y));

	renderDevice->ClearState();
	
	QuadHelper.Render();
	
	Renderer::RenderTargetBinding* target[] = {
		&Renderer::RenderTargetBinding::BackBufferRT,
		baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("pastColor")),
		baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("pastDepth")),
	};
	//target[0] = nullptr;
	renderDevice->SetRenderTargets(target, 3, nullptr);
	renderDevice->VerifyPixelSampler(0, Samplers::pointClamp);
	renderDevice->VerifyPixelTexture(0, TAAHistory);
	renderDevice->VerifyPixelTexture(1, baseRendererParams.renderSystem.texturesManger->depthBuffer->texture);
	PipelineFactoryFlags flagsCopy;
	flagsCopy.definesFlags =TAACOPY;
	renderDevice->ApplyPipelineState(factory->GetState(flagsCopy));
	renderDevice->DrawIndexedPrimitives(Renderer::PrimitiveType::PRIMITIVETYPE_TRIANGLESTRIP, 0, 0, 0, 0, 2);

	renderDevice->VerifyPixelTexture(1, nullptr);

	RenderImGUI();
}



void TAARenderPass::RenderImGUI()
{
    static bool opened = true;
    ImGui::Begin("TAA settings.", &opened);                          // Create a window called "Hello, world!" and append into it.
    //
    //ImGui::SliderFloat("depthThreshold",     &localBuffer.depthThreshold ,0.0, 1.0 , "%.10f");
    ImGui::SliderInt("NumSamples",				    &Settings.numSamples , 1, 20 , "%.3f");
    ImGui::SliderFloat("TAAShiftStrength",          &Settings.taaStrength, 0.0, 100.0  , "%.3f");
    ImGui::Checkbox("markNoHistoryPixels	    ",  &Settings.markNoHistoryPixels		    );
    ImGui::Checkbox("allowBicubicFilter		    ",  &Settings.allowBicubicFilter		    );
    ImGui::Checkbox("allowDepthThreshold	 	",  &Settings.allowDepthThreshold		    );
    ImGui::Checkbox("allowVarianceClipping	    ",  &Settings.allowVarianceClipping		);
    ImGui::Checkbox("allowNeighbourhoodSampling",   &Settings.allowNeighbourhoodSampling	);
    ImGui::Checkbox("allowYCoCg				    ",  &Settings.allowYCoCg					);
    ImGui::Checkbox("allowLongestVelocityVector",   &Settings.allowLongestVelocityVector	);
    
    ImGui::End();
}
void TAARenderPass::PostRender()
{
}
