#pragma 
#include "PPConstBuffer.h"
#include "CoreRenderSystem/RenderPasses/QuadRenderPass.h"


enum  PPRendererDefines : std::uint32_t {
	PPZERO = 0,
	PPRED = 1,
	PPALPHA = 2,
	PPSINGLETEXTURE = 4,

};

class PPRenderPass : public BaseRenderPass {
	struct PPRendererProvider;
	QuadRenderPass<0> quad;
	Renderer::Texture* skyTexture = nullptr;

	struct PPRenderPassProvider : public Renderer::IStateProvider {
		PPRenderPassProvider();
		virtual void PatchPipelineState(Renderer::Pipeline* refToPS, uint32_t definesFlags) override;
		virtual  Renderer::InputLayoutDescription GetInputLayoutDescription(size_t definesFlags) override;
		virtual const char* GetShaderName() override;
		Renderer::PipelineFactoryDescription GetFactoryDescription() override;
	};


public:

	explicit PPRenderPass(BaseRenderSystem& in);


public:
	void Resize() override {};
	void Init(const char* dirr) override;
	void Render();
	void PreRender() override;
	void PostRender() override;
	~PPRenderPass() override;

private:
	void RenderIMGUI();

	Renderer::SamplerState diffuseSampler;
	Renderer::SamplerState sampler;

	PPCosntBuffer localData;
	Renderer::ConstBuffer* pConstBuffer;

	Renderer::Texture* outTexture;
	bool ShowDebugTexture;
	std::vector<const char*> texturesList;
	std::vector<uint8_t> texutersFlags;
	

	
};

