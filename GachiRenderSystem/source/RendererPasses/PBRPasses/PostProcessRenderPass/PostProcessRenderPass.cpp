#include "PostProcessRenderPass.h"

#include "PostProcessRenderPassProvider.h"
#include "imgui/imgui.h"
#include "ResourceManagers/States/Samplers.h"


PostProcessRenderPass::PostProcessRenderPass(BaseRenderSystem& System)
        : GachiBasePass({"PBRToneMapping.hlsl", System}), QuadHelper(System.pRenderer)
{
	
	uint32_t width, height;
	renderDevice->GetMainViewportSize(width, height);
    
    PostProcessHistory = renderDevice->CreateUATexture2D(Renderer::SURFACEFORMAT_VECTOR4,width, height,1);
	constBuffer = renderDevice->CreateConstBuffer(sizeof(localBuffer));
}


void PostProcessRenderPass::Init(const char* dirr)
{
    BaseRenderPass::Init(dirr, new PostProcessRenderPassProvider());
    
}
void PostProcessRenderPass::SetupSettings(PostProcessSettings Settings)
{
	this->Settings = Settings;
}

void PostProcessRenderPass::PreRender()
{	
	uint32_t width, height;
	renderDevice->GetMainViewportSize(width, height);
	
    localBuffer.lum = Settings.lum;
}

void PostProcessRenderPass::Resize()
{
	uint32_t width, height;
	renderDevice->GetMainViewportSize(width, height);
	
    renderDevice->AddDisposeTexture(PostProcessHistory);
	PostProcessHistory = renderDevice->CreateUATexture2D(Renderer::SURFACEFORMAT_VECTOR4,width, height,1);
}

void PostProcessRenderPass::Render()
{
	uint32_t width, height;
	renderDevice->GetMainViewportSize(width, height);
	QuadHelper.Render();
	
	Renderer::RenderTargetBinding* target[] = {
		baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("preAAcolor")),
	};
	//target[0] = nullptr;
	renderDevice->SetRenderTargets(target, std::size(target), nullptr);
	
	renderDevice->VerifyPixelSampler(0, Samplers::pointClamp);
	renderDevice->VerifyPixelTexture(0, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Light"))->texture);
	renderDevice->VerifyPixelTexture(1, baseRendererParams.renderSystem.texturesManger->depthBuffer->texture);
	
	PipelineFactoryFlags flagsCopy;
	flagsCopy.definesFlags = ((1<<Settings.mode))>>1;
	renderDevice->ApplyPipelineState(factory->GetState(flagsCopy));
	//renderDevice->ApplyIndexBufferBinding(nullptr,16);
	
	renderDevice->DrawIndexedPrimitives(Renderer::PrimitiveType::PRIMITIVETYPE_TRIANGLELIST, 0, 0, 0, 0, 2);

	renderDevice->VerifyPixelTexture(1, nullptr);

	//RenderImGUI();
}



void PostProcessRenderPass::RenderImGUI()
{
    static bool opened = true;
    ImGui::Begin("PostProcess settings.", &opened);                          // Create a window called "Hello, world!" and append into it.
    
    //ImGui::SliderFloat("depthThreshold",     &localBuffer.depthThreshold ,0.0, 1.0 , "%.10f");
	static int temp = Settings.mode;
	
	ImGui::SliderInt("NumSamples",				   &temp , 1, 5 );
	Settings.mode = temp;
    
    ImGui::End();
}

void PostProcessRenderPass::PostRender()
{
}
void PostProcessRenderPass::SetupSettings(const RenderSettings& inSettings)
{
	this->Settings = inSettings.ppSettings;
}
