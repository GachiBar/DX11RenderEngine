#pragma once
#include "SSAORendererConstBuffer.h"
#include "CoreRenderSystem/RenderPasses/QuadRenderPass.h"
#include "CoreRenderSystem/BaseRenderSystem.h"

enum  SSAORendererDefines : std::uint32_t {
	SSAOZERO = 0,
	SSAORED = 1,
	SSAOBLUR = 2,
	SSAOOCCLUSION = 4,
	SSAOCOPY = 8,

};


class SSAORenderPass:public BaseRenderPass {
	QuadRenderPass<0> QuadHelper;
	struct SSAORenderPassProvider;

	Renderer::Texture* skyTexture = nullptr;

	struct SSAORenderPassProvider : public Renderer::IStateProvider {
		SSAORenderPassProvider();
		virtual void PatchPipelineState(Renderer::Pipeline* refToPS, size_t definesFlags) override;
		virtual  Renderer::InputLayoutDescription GetInputLayoutDescription(size_t definesFlags) override;
		virtual const char* GetShaderName() override;
		Renderer::PipelineFactoryDescription GetFactoryDescription() override;
	};


public:


	SSAORenderPass(BaseRenderSystem& System);

	virtual void Init(const char* dirr) override;

	void Render() ;
	
	void Resize() override {};
	void PreRender() override;
	void PostRender() override;

	
	~SSAORenderPass() override;
private:
	
	Renderer::ConstBuffer* constBuffer;
	SSAOCosntBuffer localBuffer;
	
	Renderer::Texture* randVect;

	
	Renderer::Texture* buff;
	Renderer::RenderTargetBinding  buffRT;
	
	void RenderIMGUI();
public:
	
};

