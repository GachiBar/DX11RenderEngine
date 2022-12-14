#pragma once
#include "Utils/DrawData.h"
#include "Utils/ModelMesh.h"
#include "RenderFlags.h"
#include "TextureData.h"
#include "RenderSettings.h"
#include "imgui/imgui.h"
#include "Utils/OpaqueModelDrawData.h"

class RenderSystem;


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



struct RenderDevice {

	RenderSystem* gfx = nullptr;

	/* Init */

	
	void CreateDevice(RenderEngineCoreSettings);

	/* Quit */

	void DestroyDevice();


	enum class ShaderType {
		UIShader,
		ModelsShader,
		UPShader,
		ParticlesShader,
		SkyShader,
		EndBSPShader,
		BloomShader,
		FXAAShader,
		MBShader,
	};

	struct ShaderData {
		ShaderType type;
		uint8_t* data;
		size_t dataSize;
	};
	std::string shadersDir;
	void InitDevice(const RenderSettings& Settings);
	//void ReloadShaders(LPCWSTR);
	void ReloadShaders();

	/* Presentation */


	void BeginFrame();
	void SetRenderData(const RenderData& data);
	bool Present();
	void EndFrame();
	void Clear(float r, float g, float b);

	/* Resourse loading*/
#ifdef WIN32
	void RegisterTexture(size_t id, LPCWCH file);
#endif
	void RegisterTexture(size_t id, const char* file);

	void RegisterTexture(size_t id, const TextureData&);
	void RegisterTexture(size_t id, const FloatData&);
	void RegisterTexture(size_t id, const Float3Data&);
	
	void RegisterTexture(size_t id, int width, int height, void* data);
	void UpdateTexture(size_t id, const TextureData&);
	void UpdateTexture(const ImageUpdate &);


	void RegisterModel(size_t id, const ModelMesh&);
	void RegisterModel(size_t id, const OpaqueMesh&);
	void ReleaseTexture(size_t id);
	bool WasIdUsed(size_t id);
	bool IsMeshIdUsed(size_t id);
	bool IsTextureIdUsed(size_t id);


	/* Drawing */

	void DrawColor(const UIDrawData& data);
	void DrawImg(size_t texId, const UIDrawData& data);
	
	
	
	void DrawModel(const ModelDrawData& drawData);
	void DrawOpaqueModel(const OpaqueModelDrawData& drawData);;
	void DrawLight(const LightDrawData& drawData);;
	
	
	void DrawUserPolygon(MeshHashData model, size_t textureId, UPDrawData data);
	void DrawUserPolygon(MeshHashData model, size_t textureId, size_t lightmapId, UPDrawData data);
	void DrawSetUserPolygon(MeshHashData model, UPModelMesh newModel, size_t textureId, UPDrawData data);
	MeshHashData RegisterUserPolygon(UPModelMesh model, bool dynamic);

	void DrawParticles(const ParticlesMesh& particles, const ParticlesDrawData& data);
	


	void DrawDebug(const DebugDraw3DData& drawData);
	void DrawDebug(const DebugDraw2DData& drawData);
	
	void SetSky(size_t side, const TextureData& data);
	void SetSkyFlags(uint64_t flags);

	struct Texture
	{
		void* texture;
		uint32_t width;
		uint32_t height;
	};
	Texture GetTexture(size_t textureId);
	Texture GetRenderTargetTexture(const char*);

	void ResizeBackBuffer(uint32_t width, uint32_t height);
	void ResizeViewport(uint32_t width, uint32_t height);
	std::vector<const char*> GetRenderTargetsList();

	void Flush();
	/* Debug staff */

	bool ProcessMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

};






