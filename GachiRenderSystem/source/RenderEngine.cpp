#include "pch.h"
#include "RenderEngine.h"
#include "RenderSystem.h"


void RenderDevice::CreateDevice(RenderEngineCoreSettings init) {
	if (gfx != nullptr) DestroyDevice();
	gfx =  RenderSystem::Initialise(init);
	//gfx->skyFlags = SKYNOTHING;
	//gfx->SetCameraPosition({ float3(0,0,0), float3(0,0,0), float3(1,1,1) });


	//ImGui_ImplWin32_Init(hWnd);

}

void RenderDevice::InitDevice(const RenderSettings& Settings) {
	shadersDir = Settings.shadersDirr;
	gfx->SetupSettings(Settings);
	gfx->ReloadShaders(shadersDir.c_str());
	
}
void RenderDevice::ReloadShaders()
{
	gfx->ReloadShaders(shadersDir.c_str());
}


void RenderDevice::BeginFrame() {
	gfx->BeginFrame();
}

void RenderDevice::SetRenderData(const RenderData& data){
	gfx->SetRenderData(data);
}

bool RenderDevice::Present() {
	return gfx->RenderFrame();
}

void RenderDevice::EndFrame()
{
	gfx->EndFrame();
}

void RenderDevice::SetSky(size_t side, const TextureData& data) {
	//gfx->managerSkybox.UpdateSkybox(data, side);
}

void RenderDevice::SetSkyFlags(uint64_t flags) {
	//gfx->skyFlags = flags;
}

RenderDevice::Texture RenderDevice::GetTexture(size_t textureId)
{
	auto hTexture  = gfx->texturesManger->GetImg(textureId);
	return {gfx->pRenderer->GetNativeTexture(hTexture.texture), hTexture.width, hTexture.height};
}

RenderDevice::Texture RenderDevice::GetRenderTargetTexture(const char* name)
{
	auto hRT  = gfx->texturesManger->GetRenderTarget(SIDRT(name));
	auto hDesc  = gfx->texturesManger->GetRenderTargetDescription(SIDRT(name));
	return {gfx->pRenderer->GetNativeTexture(hRT->texture), hDesc.width, hDesc.height};
}

void RenderDevice::ResizeBackBuffer(uint32_t width, uint32_t height)
{
	gfx->ResizeBackBuffer(width, height);
}

void RenderDevice::ResizeViewport(uint32_t width, uint32_t height)
{
	gfx->ResizeViewport(width, height);
}

std::vector<const char*> RenderDevice::GetRenderTargetsList()
{
	return  gfx->texturesManger->GetRenderTargetsList();
}

void RenderDevice::Flush() {
	//gfx->PostRender();
}

void RenderDevice::Clear(float r, float g, float b) {
	gfx->ClearBuffer(sm::Vector4(r, g, b, 1));
}

void RenderDevice::RegisterTexture(size_t id, LPCWCH file) {

	gfx->RegisterImg(id, TextureData::FromFile(file));
}

void RenderDevice::RegisterTexture(size_t id, const char* file) {

	gfx->RegisterImg(id, TextureData::FromFile(file));
}

void RenderDevice::RegisterTexture(size_t id, const TextureData& tex) {
	gfx->RegisterImg(id, tex);
}
void RenderDevice::RegisterTexture(size_t id, const FloatData& tex)
{
	gfx->RegisterImg(id, tex);
}
void RenderDevice::RegisterTexture(size_t id, const Float3Data& tex)
{
	gfx->RegisterImg(id, tex);
}

void RenderDevice::RegisterTexture(size_t id, int width, int height, void* data) {
	gfx->RegisterImg(id, width, height, data);
}

void RenderDevice::UpdateTexture(size_t id, const TextureData& tex) {
	gfx->UpdateImg(id, tex);
}

void RenderDevice::UpdateTexture(const ImageUpdate& data) {
	gfx->UpdateImg(data);
}

void RenderDevice::RegisterModel(size_t id, const ModelMesh& model) {
	gfx->RegisterModel(id, model);
}

void RenderDevice::RegisterModel(size_t id, const OpaqueMesh& model)
{
	gfx->RegisterOpaqueModel(id, model);
}

void RenderDevice::ReleaseTexture(size_t id) {
	gfx->ReleaseImg(id);
}

bool RenderDevice::WasIdUsed(size_t id)
{
	return gfx->WasIdUsed(id);
}

bool RenderDevice::IsTextureIdUsed(size_t id)
{
	return gfx->IsTextureIdUsed(id);
}

bool RenderDevice::IsMeshIdUsed(size_t id)
{
	return gfx->IsMeshIdUsed(id);
}

void RenderDevice::DrawColor(const UIDrawData& data) {
	UIDrawData buff(data);
	buff.flag |= UICOLORED;
	gfx->DrawColor(buff);
}

void RenderDevice::DrawImg(size_t texId, const UIDrawData& data) {
	gfx->DrawImg(texId, data);
}

void RenderDevice::DrawModel(const ModelDrawData& drawData)
{
	gfx->DrawModel(drawData);
}
void RenderDevice::DrawOpaqueModel(const OpaqueModelDrawData& drawData)
{
	gfx->DrawOpaqueModel(drawData);
}
void RenderDevice::DrawLight(const LightDrawData& drawData)
{
	gfx->DrawLight(drawData);
}

void RenderDevice::DrawUserPolygon(MeshHashData model, size_t textureId, UPDrawData data) {
	gfx->DrawUserPolygon(model, textureId, data);
}

void RenderDevice::DrawUserPolygon(MeshHashData model, size_t textureId, size_t lightmapId, UPDrawData data) {
	gfx->DrawUserPolygon(model, textureId, lightmapId, data);
}

void RenderDevice::DrawSetUserPolygon(MeshHashData model, UPModelMesh newModel, size_t textureId, UPDrawData data) {
	gfx->DrawSetUserPolygon(model,newModel, textureId, data);
}

MeshHashData RenderDevice::RegisterUserPolygon(UPModelMesh model, bool dynamic) {
	return gfx->RegisterStaticPolygon(model, dynamic);
}

void RenderDevice::DrawParticles(const ParticlesMesh& particles, const ParticlesDrawData& data) {
	gfx->DrawParticles(particles, data);
}

void RenderDevice::DrawDebug(const DebugDraw3DData& drawData)
{
	gfx->DrawDebug(drawData);
}

void RenderDevice::DrawDebug(const DebugDraw2DData& drawData)
{
	gfx->DrawDebug(drawData);
}


bool RenderDevice::ProcessMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
    }
	return false;
}

void RenderDevice::DestroyDevice() {

	//PixelShader2D::Release();
	//VertexShader2D::Release();
	//ImGui_ImplWin32_Shutdown();
	if (gfx) delete gfx;
	gfx = nullptr;
}
