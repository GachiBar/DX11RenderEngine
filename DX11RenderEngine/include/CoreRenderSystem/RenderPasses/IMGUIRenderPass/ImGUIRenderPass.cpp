
#include "ImGUIRenderPass.h"
#include "SimpleMath.h"

ImGUIRenderPass::ImGUIRenderPass(const BaseRendererParams& in): BaseRenderPass(in) {}

void ImGUIRenderPass::Init(const char*) {}

void ImGUIRenderPass::Resize()
{
}

void ImGUIRenderPass::PreRender() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGUIRenderPass::Render() {
}


void ImGUIRenderPass::PostRender()
{
	renderDevice->SetRenderTargets(nullptr, 0,nullptr,
{});  
	static bool show = true;
	ImGui::ShowDemoWindow(&show);
	
	ImGuiIO& io = ImGui::GetIO();
	
	
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
	
}

ImGUIRenderPass::~ImGUIRenderPass() {}

