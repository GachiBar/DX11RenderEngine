#pragma once
#include "CoreRenderSystem/BaseRenderSystem.h"
#include "SimpleMath.h"

#include "CoreRenderSystem/RenderPasses/IMGUIRenderPass/ImGUIRenderPass.h"
#include "RendererPasses/ModelsRenderPass/ModelsRenderPass.h"
#include "RendererPasses/PPRenderPass/PPRenderPass.h"
#include "RendererPasses/SSAORenderPass/SSAORenderPass.h"
#include "RendererPasses/TAARenderPass/TAARenderPass.h"
#include "RendererPasses/UIRenderPass/UIRenderPass.h"
#include "RendererPasses/UPRenderPass/UPRenderPass.h"
#include "ResourceManagers/ModelsManager.h"
#include "Utils/DrawData.h"


#include "Utils/ModelData.h"

namespace Renderer {
struct ConstBuffer;
}

namespace sm = DirectX::SimpleMath;

class RenderSystem: public BaseRenderSystem{



	RenderSystem(RenderEngineInitStruct,
		const BaseRenderSystemInitStruct&,
		ModelsManager* modelsManager,
		TexturesManager* texturesManager);
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
public:
	static RenderSystem* Initialise(RenderEngineInitStruct);
	~RenderSystem() override;


	void BeginFrame();
	bool RenderFrame();
	void EndFrame();
	void ClearBuffer(sm::Vector4 color) noexcept;
	void PostRender();



	void RegisterImg(size_t id, const TextureData& text);
	void RegisterImg(size_t id, int width, int heights, void* data, bool mipmap);

	void UpdateImg(size_t id, const TextureData& text);
	void UpdateImg(const ImageUpdate&);
	void ReleaseImg(size_t id);

	void DrawColor(const UIDrawData& data);
	void DrawImg(size_t texId, const UIDrawData& data);


	void RegisterModel(size_t id, const ModelData& model);
	void RegisterFramedModel(size_t id, const FramedModelData& model);
	void ReleaseModel(size_t id);




	void DrawModel(size_t modelId, size_t textureId, Transform position, size_t flags);


	void DrawUserPolygon(MeshHashData model, size_t textureId, UPDrawData data);
	void DrawUserPolygon(MeshHashData model, size_t textureId, size_t lightmapId, UPDrawData data);
	void DrawSetUserPolygon(MeshHashData model, UPModelMesh newModel, size_t textureId, UPDrawData data);
	MeshHashData RegisterhUserPolygon(UPModelMesh model, bool dynamic);



	void DrawFramedModel(size_t modelId, size_t textureId, const LerpModelDrawData& data);


	void DrawParticles(const ParticlesMesh& particles, const ParticlesDrawData& data);

	
	void ReinitShaders(const char *);
	
	//SkyboxRenderer	     managerSkybox;
private:

	ModelsManager* modelsManager;
	TexturesManager* texturesManager;
	
	std::vector<BaseRenderPass*> renderPasses;
	
	UIRenderPass			 renderPassUI;
	ModelsRenderPass		 renderPassModels;
	ImGUIRenderPass			 renderPassIMGUI;
	TAARenderPass			 renderPassTAA;
	UPRenderPass			 renderPassUP;
	PPRenderPass			 renderPassPP;
	SSAORenderPass			 renderPassSSAO;
	
	//MotionBlurRenderer	 managerMB;
	//PPRenderer			 managerPostProcess;
	//ParticlesRenderer	 managerParticles;
	//BloomRenderer	     managerBloom;
	//FXAARenderer	     managerFXAA;
	//TAARenderer	         managerTAA;
	//SSAORenderer	     managerSSAO;


};