#include "pch.h"


#include "RenderSystem.h"

#include "D3D11Renderer.h"

#include <filesystem>
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")


using namespace Renderer;

RenderSystem::RenderSystem(RenderEngineCoreSettings init, const BaseRenderSystemInitStruct& bInit,
                           ModelsManager* modelsManager,
                           TexturesManager* texturesManager) :
    BaseRenderSystem(bInit),
    renderPassUI(*this),
    renderPassModels(*this),
    renderPassIMGUI({"", *this}),
    renderPassTAA(*this),
    renderPassDebug(*this),
    renderPassOpaque(*this),
    renderPassLight(*this),
    renderPassPP(*this),
    modelsManager(modelsManager),
    texturesManager(texturesManager)
{
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplWin32_Init(init.hWnd);
    ImGui_ImplDX11_Init((ID3D11Device*)((D3D11Renderer*)pRenderer)->GetDevice(),(ID3D11DeviceContext*)((D3D11Renderer*)pRenderer)->GetContext());

    uint32_t w,h;
    pRenderer->GetBackbufferSize(w,h);
    Viewport vp {0,0,(int32_t)w,(int32_t)h, 0,1};
    pRenderer->SetBackBufferViewport(vp);
    
    baseRenderPasses.push_back(&renderPassIMGUI);

    gachRenderPasses.push_back(&renderPassUI);
    gachRenderPasses.push_back(&renderPassModels);
    gachRenderPasses.push_back(&renderPassTAA);
    gachRenderPasses.push_back(&renderPassDebug);
    gachRenderPasses.push_back(&renderPassOpaque);
    gachRenderPasses.push_back(&renderPassLight);
    gachRenderPasses.push_back(&renderPassPP);
    //managerImGUI.Init();
    //ImGui_ImplDX11_Init(pRenderer.device.Get(), pRenderer.context.Get());7

    //pLocalConstants = pRenderer->CreateConstBuffer(sizeof(taaConstants));
    //taaConstants.taaStrength = 1;
    //pRenderer->VerifyConstBuffer(pLocalConstants, taaShiftBuffer.slot);
    ////
    //	//renderPasses.push_back(&managerSkybox);
    //	//renderPasses.push_back(&managerUI);
    //	//renderPasses.push_back(&managerModels);
    //	//renderPasses.push_back(&managerMB);
    //	//renderPasses.push_back(&managerUP);
    //	//renderPasses.push_back(&managerPostProcess);
    //	//renderPasses.push_back(&managerParticles);
    //	//renderPasses.push_back(&managerBloom);
    //	//renderPasses.push_back(&managerFXAA);
    //	//renderPasses.push_back(&managerTAA);
    //	//renderPasses.push_back(&managerSSAO);
    //
}

void RenderSystem::SetupSettings(const RenderSettings& Settings)
{
    for (auto pass : gachRenderPasses)
    {
        pass->SetupSettings(Settings);
    }
}

void RenderSystem::ReloadShaders(const char* shadersDirr)
{
    for (auto pass : baseRenderPasses)
    {
        pass->Init(shadersDirr);
    }

    for (auto pass : gachRenderPasses)
    {
        pass->Init(shadersDirr);
    }
}

void RenderSystem::DrawDebug(const DebugDraw3DData& drawData)
{
#if _DEBUG
    renderPassDebug.Draw(drawData);
#endif
}

void RenderSystem::DrawDebug(const DebugDraw2DData& drawData)
{
#if _DEBUG
    renderPassDebug.Draw(drawData);
#endif
}

void RenderSystem::ResizeBackBuffer(uint32_t width, uint32_t height)
{
    pRenderer->ResizeBackbuffer(Size2D(width, height));
    texturesManager->ResizeTextures();
    Resize();
}

void RenderSystem::ResizeViewport(uint32_t width, uint32_t height)
{
    pRenderer->ResizeMainViewport(Size2D(width, height));
    texturesManager->ResizeTextures();
    Resize();
}
void RenderSystem::DrawOpaqueModel(const OpaqueModelDrawData& drawData)
{
    renderPassOpaque.Draw(drawData);
}
void RenderSystem::DrawLight(const LightDrawData& light)
{
    renderPassLight.Draw(light);
}

void RenderSystem::Resize()
{
    for (auto pass : baseRenderPasses)
    {
        pass->Resize();
    }

    for (auto pass : gachRenderPasses)
    {
        pass->Resize();
    }
};

RenderSystem* RenderSystem::Initialise(RenderEngineCoreSettings init)
{
    auto renderDevice = new Renderer::D3D11Renderer{
        {
            {
                (int32_t)init.windowSettings.windowWidth,
                (int32_t)init.windowSettings.windowHeight
            },
            {
                (int32_t)init.windowSettings.windowWidth,
                (int32_t)init.windowSettings.windowHeight
            },
            0,
            init.hWnd,
            false,
            DepthFormat::DEPTHFORMAT_D32,
            PresentInterval::PRESENTINTERVAL_DEFAULT
        },
        1
    };

    auto t = new TexturesManager(renderDevice);
    auto m = new ModelsManager(renderDevice);
    return new RenderSystem{
        init,
        {
            renderDevice, t, m
        },
        m, t
    };
    //todo
}

void RenderSystem::BeginFrame()
{
    pRenderer->ClearState();
    for (auto pass : baseRenderPasses)
    {
        pass->PreRender();
    }
    for (auto pass : gachRenderPasses)
    {
        pass->PreRender();
    }
}

#define GFX_CATCH_RENDER(render) try {render;} catch (const std::exception& exe) {printf_s(exe.what()); printf_s("\n"); static char c[100]; scanf_s("%s", c,1); success = false; }
//#define GFX_CATCH_RENDER(render) {render}

bool RenderSystem::RenderFrame()
{

    
//#if _DEBUG
//    renderPassDebug.Draw(
//        DebugDraw3DData
//        {
//            {
//                EPrimitiveType::PRIMITIVETYPE_LINESTRIP,
//                4,
//                {
//                    DebugVertex3D{{0, 0, -10}},
//                    DebugVertex3D{{3, 0, -10}},
//                    DebugVertex3D{{6, 0, -10}},
//                    DebugVertex3D{{20, 40, -10}},
//                    DebugVertex3D{{100, 100, -10}}
//                },
//                {(uint32_t)0, (uint32_t)1, (uint32_t)2, (uint32_t)3, (uint32_t)4},
//            },
//            Transform(),
//            float3(0, 1, 0),
//            {}
//        }
//    );
//
//#endif


    pRenderer->ClearState();
    BaseRenderSystem::Present();

    //re.UpdateHaltonSequence();

    //pRenderer->SetConstBuffer(pLocalConstants, &taaConstants);
    // pRenderer->VerifyConstBuffer(pLocalConstants, taaShiftBuffer.slot);

    BaseRenderSystem::Present();

    bool success = true;
    pRenderer->BeginEvent("BSP draw.");
    //GFX_CATCH_RENDER(managerUP.Render(*this););
    pRenderer->EndEvent();

    pRenderer->BeginEvent("BloomMask draw.");
    //GFX_CATCH_RENDER(managerBloom.RenderBloomMask(*this););
    pRenderer->EndEvent();

    pRenderer->BeginEvent("Static motion blur draw.");
    //GFX_CATCH_RENDER(managerMB.RenderStatic(*this););
    pRenderer->EndEvent();

    pRenderer->BeginEvent("Models draw.");
    GFX_CATCH_RENDER(renderPassModels.Render());
    pRenderer->EndEvent();

    
    pRenderer->BeginEvent("Opaque models draw.");
    renderPassOpaque.Render(); 
    pRenderer->EndEvent();


    pRenderer->BeginEvent("Light draw.");
    GFX_CATCH_RENDER(renderPassLight.Render());
    pRenderer->EndEvent();

    //pRenderer->BeginEvent("Dynamic motion blur draw.");
    //GFX_CATCH_RENDER(managerMB.RenderDynamic(*this););
    //pRenderer->EndEvent();


    pRenderer->BeginEvent("SSAO draw.");
    //GFX_CATCH_RENDER(managerSSAO.Render(*this););
    pRenderer->EndEvent();

    pRenderer->BeginEvent("Sky draw.");
    //GFX_CATCH_RENDER(managerSkybox.Render(*this););
    pRenderer->EndEvent();

    pRenderer->BeginEvent("Particles draw.");
    //GFX_CATCH_RENDER(managerParticles.Render(*this););
    pRenderer->EndEvent();

    pRenderer->BeginEvent("Bloom pass.");
    //GFX_CATCH_RENDER(managerBloom.Render(*this););
    pRenderer->EndEvent();

    pRenderer->BeginEvent("PostProcess.");
    GFX_CATCH_RENDER(renderPassPP.Render(););
    pRenderer->EndEvent();

    pRenderer->BeginEvent("TAA-pass.");
    renderPassTAA.Render();
    pRenderer->EndEvent();

    pRenderer->BeginEvent("UI draw.");
    renderPassUI.Render();
    pRenderer->EndEvent();

    BaseRenderSystem::Present();
#if _DEBUG
    pRenderer->BeginEvent("Debug draw.");
    GFX_CATCH_RENDER(renderPassDebug.Render());
    pRenderer->EndEvent();
#endif

    pRenderer->BeginEvent("IMGUI draw.");
    renderPassIMGUI.Render();
    pRenderer->EndEvent();

    pRenderer->ClearState();
    

    GFX_CATCH_RENDER(pRenderer->RunVM());

    
    return success;
}

void RenderSystem::EndFrame()
{
    PostRender();

    pRenderer->SwapBuffers();
}

void RenderSystem::ClearBuffer(sm::Vector4 color) noexcept
{
    pRenderer->SetRenderTargets(nullptr, 0, texturesManger->depthBuffer);
    pRenderer->Clear((ClearOptions)7, {color.x, color.y, color.z, color.w}, 1, 0u);
    //pContext->ClearRenderTargetView(pTarget.Get(), reinterpret_cast<float*>(&color));
    //pContext->ClearDepthStencilView(pDSV.Get(), DModels11_CLEAR_DEPTH, 1.0f, 0u);
}

void RenderSystem::PostRender()
{
    //managerUP.Flush();

    for (auto pass : baseRenderPasses)
    {
        pass->PostRender();
    }
    for (auto pass : gachRenderPasses)
    {
        pass->PostRender();
    }
}


void RenderSystem::DrawImg(size_t texId, const UIDrawData& data)
{
    //queue.push_back(DrawCall::draw2D).
    renderPassUI.Draw(texturesManger->GetImg(texId), data);
}


void RenderSystem::DrawColor(const UIDrawData& data)
{
    renderPassUI.Draw(data);
}

void RenderSystem::RegisterImg(size_t id, const TextureData& text)
{
    texturesManger->RegTexture(text, id);
}
void RenderSystem::RegisterImg(size_t id, const FloatData& text)
{
    texturesManger->RegFloatTexture(text, id);
}
void RenderSystem::RegisterImg(size_t id, const Float3Data& text)
{
    texturesManger->RegFloat3Texture(text, id);
}

void RenderSystem::UpdateImg(size_t id, const TextureData& text)
{
    texturesManger->UpdateTexture(text, id);
}

void RenderSystem::UpdateImg(const ImageUpdate& data)
{
    texturesManger->UpdateTexture(data);
}

void RenderSystem::ReleaseImg(size_t id)
{
    texturesManger->ReleaseTexture(id);
}

void RenderSystem::RegisterModel(size_t id, const ModelMesh& model)
{
    modelsManager->RegisterModel(id, model);
}
void RenderSystem::RegisterOpaqueModel(size_t id, const OpaqueMesh& model)
{
    modelsManager->RegisterModel(id, model);
}

void RenderSystem::ReleaseModel(size_t id)
{
    modelsManager->ReleaseModel(id);
}

void RenderSystem::RegisterImg(size_t id, int width, int height, void* data)
{
    texturesManger->RegTexture(data, width, height, id);
}

bool RenderSystem::WasIdUsed(size_t id)
{
    return IsTextureIdUsed(id) || IsMeshIdUsed(id);
}

bool RenderSystem::IsTextureIdUsed(size_t id)
{
    return texturesManger->WasIdUsed(id);
}

bool RenderSystem::IsMeshIdUsed(size_t id)
{
    return modelsManager->WasIdUsed(id);
}

void RenderSystem::DrawModel(const ModelDrawData& drawData)
{
    renderPassModels.Draw(drawData);
}

void RenderSystem::DrawUserPolygon(MeshHashData model, size_t textureId, UPDrawData data)
{
    //managerUP.Draw(model, texturesManger->GetImg(textureId), data);
}

void RenderSystem::DrawUserPolygon(MeshHashData model, size_t textureId, size_t lightmapId, UPDrawData data)
{
    //managerUP.Draw(model, texturesManger->GetImg(textureId), texturesManger->GetImg(lightmapId), data);
}

void RenderSystem::DrawSetUserPolygon(MeshHashData model, UPModelMesh newModel, size_t textureId, UPDrawData data)
{
    //managerUP.DrawSet(model, newModel, texturesManger->GetImg(textureId), data);
}

MeshHashData RenderSystem::RegisterStaticPolygon(UPModelMesh model, bool dynamic)
{
    return {};
    //return managerUP.Register(model, dynamic);
}

void RenderSystem::DrawParticles(const ParticlesMesh& particles, const ParticlesDrawData& data)
{
    // managerParticles.Draw(particles, data);
}
