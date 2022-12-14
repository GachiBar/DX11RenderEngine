#pragma once
#include "RenderSettings.h"
#include "CoreRenderSystem/BaseRenderSystem.h"
#include "SimpleMath.h"

#include "CoreRenderSystem/RenderPasses/IMGUIRenderPass/ImGUIRenderPass.h"
#include "RendererPasses/DebugRenderPass/DebugRenderPass.h"
#include "RendererPasses/ModelsRenderPass/ModelsRenderPass.h"
#include "RendererPasses/PBRPasses/OpaqueRenderPass/OpaqueRenderPass.h"
#include "RendererPasses/PBRPasses/LightRenderPass/LightRenderPass.h"
#include "RendererPasses/PBRPasses/PostProcessRenderPass/PostProcessRenderPass.h"
#include "RendererPasses/TAARenderPass/TAARenderPass.h"
#include "RendererPasses/UIRenderPass/UIRenderPass.h"
#include "ResourceManagers/ModelsManager.h"
#include "Utils/DrawData.h"


#include "Utils/ModelMesh.h"

namespace Renderer {
struct ConstBuffer;
}

namespace sm = DirectX::SimpleMath;

class RenderSystem: public BaseRenderSystem{



	RenderSystem(RenderEngineCoreSettings,
		const BaseRenderSystemInitStruct&,
		ModelsManager* modelsManager,
		TexturesManager* texturesManager);
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
public:
	static RenderSystem* Initialise(RenderEngineCoreSettings);
	~RenderSystem() = default;


	void BeginFrame();
	bool RenderFrame();
	void EndFrame();
	void ClearBuffer(sm::Vector4 color) noexcept;
	void PostRender();



	void RegisterImg(size_t id, const TextureData& text);
	void RegisterImg(size_t id, const FloatData& text);
	void RegisterImg(size_t id, const Float3Data& text);
	void RegisterImg(size_t id, int width, int heights, void* data);
	bool WasIdUsed(size_t id);
	bool IsMeshIdUsed(size_t id);
	bool IsTextureIdUsed(size_t id);

	void UpdateImg(size_t id, const TextureData& text);
	void UpdateImg(const ImageUpdate&);
	void ReleaseImg(size_t id);

	void DrawColor(const UIDrawData& data);
	void DrawImg(size_t texId, const UIDrawData& data);

	void RegisterModel(size_t id, const ModelMesh& model);
	void RegisterOpaqueModel(size_t id, const OpaqueMesh& model);
	void ReleaseModel(size_t id);

	
	void DrawModel(const ModelDrawData& drawData);
	void DrawUserPolygon(MeshHashData model, size_t textureId, UPDrawData data);
	void DrawUserPolygon(MeshHashData model, size_t textureId, size_t lightmapId, UPDrawData data);
	void DrawSetUserPolygon(MeshHashData model, UPModelMesh newModel, size_t textureId, UPDrawData data);
	MeshHashData RegisterStaticPolygon(UPModelMesh model, bool dynamic);
	
	
	void DrawParticles(const ParticlesMesh& particles, const ParticlesDrawData& data);

	
	void SetupSettings(const RenderSettings& Settings);
	void ReloadShaders(const char* dirr);
	void DrawDebug(const DebugDraw3DData& drawData);
	void DrawDebug(const DebugDraw2DData& drawData);
	void ResizeBackBuffer(uint32_t width, uint32_t height);
	void ResizeViewport(uint32_t width, uint32_t height);
	void DrawOpaqueModel(const OpaqueModelDrawData& draw_data);
	void DrawLight(const LightDrawData& light);

	//SkyboxRenderer	     managerSkybox;
private:

	void Resize();
	
	ModelsManager* modelsManager;
	TexturesManager* texturesManager;
	
	std::vector<GachiBasePass*> gachRenderPasses;
	std::vector<BaseRenderPass*> baseRenderPasses;
	
	UIRenderPass			 renderPassUI;
	ModelsRenderPass		 renderPassModels;
	ImGUIRenderPass			 renderPassIMGUI;
	TAARenderPass	         renderPassTAA;
	OpaqueRenderPass		 renderPassOpaque;
	LightRenderPass			 renderPassLight;
	PostProcessRenderPass	 renderPassPP;
	
#if _DEBUG
	DebugRenderPass	         renderPassDebug;

#endif
	//MotionBlurRenderer	 managerMB;
	//UPRenderer			 managerUP;
	//PPRenderer			 managerPostProcess;
	//ParticlesRenderer	 managerParticles;
	//BloomRenderer	     managerBloom;
	//FXAARenderer	     managerFXAA;
	//SSAORenderer	     managerSSAO;


};