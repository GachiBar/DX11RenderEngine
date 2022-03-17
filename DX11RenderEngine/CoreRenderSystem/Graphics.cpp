#include "pch.h"
#include "Graphics.h"
#include <sstream>
#include <cmath>
#include <d3dcompiler.h>
#include <filesystem>
#include <resource.h>


#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")


using namespace Renderer;

Graphics::Graphics(HWND hWnd, size_t width, size_t height)
	:GraphicsBase(hWnd, width, height)
{
	ImGui::CreateContext();
	
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_ViewportsEnable;
	
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(renderer.device.Get(), renderer.context.Get());
	
	//managerImGUI.Init();
	//ImGui_ImplDX11_Init(renderer.device.Get(), renderer.context.Get());

}

void Graphics::BeginFrame() {

	manager3D.Clear();
	managerUP.Clear(*this);
	manager2D.Clear();
	managerParticles.Clear();
}



bool Graphics::RenderFrame() {
	managerIMGUI.BeginFrame(*this);
	
	bool success = true;
	renderer.BeginEvent("BSP draw.");
	GFX_CATCH_RENDER(managerUP.Render(*this););
	renderer.EndEvent();

	renderer.BeginEvent("Motion blur draw.");
	GFX_CATCH_RENDER(managerMB.Render(*this););
	renderer.EndEvent();
	
	renderer.BeginEvent("Models draw.");
	GFX_CATCH_RENDER(manager3D.Render(*this););
	renderer.EndEvent();
	
	renderer.BeginEvent("Sky draw.");
	GFX_CATCH_RENDER(managerSkybox.Render(*this););
	renderer.EndEvent();

	renderer.BeginEvent("Particles draw.");
	GFX_CATCH_RENDER(managerParticles.Render(*this););
	renderer.EndEvent();

	renderer.BeginEvent("Bloom pass.");
	GFX_CATCH_RENDER(managerBloom.Render(*this););
	renderer.EndEvent();

	renderer.BeginEvent("End BSP draw.");
	GFX_CATCH_RENDER(managerPostProcess.Render(*this););
	renderer.EndEvent();

	
	renderer.BeginEvent("FXAA-pass.");
	GFX_CATCH_RENDER(managerFXAA.Render(*this););
	renderer.EndEvent();

	renderer.BeginEvent("UI draw.");
	GFX_CATCH_RENDER(manager2D.Render(););
	renderer.EndEvent();
	
	return success;
}

void Graphics::EndFrame()
{
	renderer.BeginEvent("IMGUI draw.");
	managerIMGUI.Render();
	renderer.EndEvent();
	renderer.SwapBuffers();
}

void Graphics::ClearBuffer(sm::Vector4 color) noexcept {
	renderer.SetRenderTargets(nullptr, 0, texturesManger.depthBuffer, Viewport());
	renderer.Clear((ClearOptions)7, { color.x, color.y, color.z, color.w }, 1, 0u);
	//pContext->ClearRenderTargetView(pTarget.Get(), reinterpret_cast<float*>(&color));
	//pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

void Graphics::Flush() {
	managerUP.Flush();
}



void Graphics::DrawImg(size_t texId, const UIDrawData& data) {
	//queue.push_back(DrawCall::draw2D).
	manager2D.Draw(texturesManger.GetImg(texId), data);
}


void Graphics::DrawColor(const UIDrawData& data) {
	manager2D.Draw(data);
}

void Graphics::RegisterImg(size_t id, const TextureData& text) {
	texturesManger.RegTexture(text, id);
}

void Graphics::UpdateImg(size_t id, const TextureData& text) {
	texturesManger.UpdateTexture(text, id);
}

void Graphics::UpdateImg(const ImageUpdate& data) {
	texturesManger.UpdateTexture(data);
}

void Graphics::ReleaseImg(size_t id) {
	texturesManger.ReleaseTexture(id);
}

void Graphics::RegisterModel(size_t id, const ModelData& model) {
	modelsManadger.RegisterModel(model, id);
}

void Graphics::RegisterFramedModel(size_t id, const FramedModelData& model) {
	modelsManadger.RegisterFramedModel(model, id);
}

void Graphics::ReleaseModel(size_t id) {
	modelsManadger.ReleaseModel(id);
}

void Graphics::RegisterImg(size_t id, int width, int height, void* data, bool mipmap) {
	texturesManger.RegTexture(data, width, height, mipmap, id);

}

void Graphics::DrawModel(size_t modelId, size_t textureId, Transform position, size_t flags) {
	manager3D.Draw(modelsManadger.GetModel(modelId), texturesManger.GetImg(textureId), position, flags);
}

void Graphics::DrawUserPolygon(MeshHashData model, size_t textureId, UPDrawData data) {
	managerUP.Draw(model, texturesManger.GetImg(textureId), data);
}
void Graphics::DrawUserPolygon(MeshHashData model, size_t textureId, size_t lightmapId, UPDrawData data) {
	managerUP.Draw(model, texturesManger.GetImg(textureId), texturesManger.GetImg(lightmapId), data);
}

void Graphics::DrawSetUserPolygon(MeshHashData model, UPModelData newModel, size_t textureId, UPDrawData data) {
	managerUP.DrawSet(model, newModel, texturesManger.GetImg(textureId), data);
}

MeshHashData Graphics::RegisterhUserPolygon(UPModelData model, bool dynamic) {
	return managerUP.Register(model, dynamic);
}

void Graphics::DrawFramedModel(size_t modelId, size_t textureId, const LerpModelDrawData& data) {
	manager3D.DrawLerp(modelsManadger.GetModel(modelId), texturesManger.GetImg(textureId), data);
}

void Graphics::DrawParticles(const ParticlesMesh& particles, const ParticlesDrawData& data) {
	managerParticles.Draw(particles, data);
}


