#include "MotionBlurRenderer.h"

#include "imgui/imgui.h"

using namespace Renderer;

MotionBlurRenderer::MotionBlurRendererProvider::MotionBlurRendererProvider(int32_t width, int32_t height) {}

void MotionBlurRenderer::MotionBlurRendererProvider::PatchPipelineState(Renderer::PipelineState* refToPS, size_t definesFlags) {

	refToPS->bs = &BlendStates::NoAlpha;
	refToPS->dss = &DepthStencilStates::NoDSS;
	refToPS->rs = &RasterizerStates::All;

}
const D3D11_INPUT_ELEMENT_DESC DefaultInputElements[] =
{
		{"Position",  0, DXGI_FORMAT_R32G32_FLOAT,  0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
};


Renderer::InputLayoutDescription MotionBlurRenderer::MotionBlurRendererProvider::GetInputLayoutDescription(size_t definesFlags) {
	return Renderer::InputLayoutDescription{ (void*)DefaultInputElements, std::size(DefaultInputElements) };
}

const char* MotionBlurRenderer::MotionBlurRendererProvider::GetShaderName()
{
	return "MotionBlur";
}


MotionBlurRenderer::MotionBlurRenderer() : QuadRenderer("MotionBlurShader.hlsl") {}

void MotionBlurRenderer::Init(void* shaderData, size_t dataSize) {
	if (provider != nullptr) {
		//delete provider;
		delete factory;
		int32_t width, height;
		renderer->GetBackbufferSize(&width, &height);
		provider = new MotionBlurRendererProvider(width, height);
		factory = new MotionBlurRendererFactory(provider, shaderData, dataSize);
		return;
	}
	QuadRenderer::Init();
	
	int32_t width, height;
	renderer->GetBackbufferSize(&width, &height);
	provider = new MotionBlurRendererProvider(width, height);
	factory = new MotionBlurRendererFactory(provider, shaderData, dataSize);
	

	localBuffer.strength	= 0.75;
	localBuffer.numSampes	= 5;
	
	constBuffer = renderer->CreateConstBuffer(sizeof(localBuffer));
}

void MotionBlurRenderer::Init(LPCWSTR dirr) {}

void MotionBlurRenderer::Render(GraphicsBase& gfx) {
	QuadRenderer::Render();
	int32_t width, height;
	renderer->GetBackbufferSize(&width, &height);

	RenderIMGUI(gfx);
	
	gfx.texturesManger.CreateRenderTarget(SURFACEFORMAT_COLOR, width, height, buffColor,buffColorRT);
	gfx.texturesManger.CreateRenderTarget(SURFACEFORMAT_COLOR, width, height, buffLight, buffLightRT);

	RenderTargetBinding* targets[] = {&buffColorRT, &buffLightRT};
	
	renderer->VerifyConstBuffer(constBuffer, motionBlurCosntBuffer.slot);
	renderer->SetConstBuffer(constBuffer, &localBuffer);

	
	renderer->SetRenderTargets(targets, 2, nullptr, vp);
	renderer->VerifyPixelSampler(0, Samplers::anisotropic16);
	renderer->VerifyPixelTexture(0, gfx.texturesManger.diffuseColor);
	renderer->VerifyPixelTexture(1, gfx.texturesManger.lightColor);
	renderer->VerifyPixelTexture(2, gfx.texturesManger.depthBuffer->texture);

	renderer->ApplyPipelineState(factory->GetState(MBZERO));
	renderer->DrawIndexedPrimitives(PrimitiveType::PRIMITIVETYPE_TRIANGLESTRIP, 0, 0, 0, 0, 2);

	
	renderer->VerifyPixelTexture(2, nullptr);
	targets[0] = &gfx.texturesManger.diffuseColorRT;
	targets[1] = &gfx.texturesManger.lightColorRT;
	
	renderer->SetRenderTargets(targets, 2, nullptr, vp);
	renderer->VerifyPixelTexture(0, buffColor);
	renderer->VerifyPixelTexture(1, buffLight);
	
	renderer->ApplyPipelineState(factory->GetState(MBCOPY));
	renderer->DrawIndexedPrimitives(PrimitiveType::PRIMITIVETYPE_TRIANGLESTRIP, 0, 0, 0, 0, 2);
}

void MotionBlurRenderer::Clear(){}


void MotionBlurRenderer::RenderIMGUI(GraphicsBase& gfx)
{	
	ImGui::Begin("Motion blur settings.");                          // Create a window called "Hello, world!" and append into it.
//
	ImGui::SliderFloat("Strength",           &localBuffer.strength		,0.0, 1.0 , "%.3f");
	ImGui::SliderInt("NumSamples",    &localBuffer.numSampes		,1, 20 , "%.3f");
	
	ImGui::End();
}


MotionBlurRenderer::~MotionBlurRenderer()
{
	renderer->AddDisposeConstBuffer(constBuffer);
	delete factory;
}
