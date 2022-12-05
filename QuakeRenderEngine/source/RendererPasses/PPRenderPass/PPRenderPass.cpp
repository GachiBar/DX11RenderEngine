#include "pch.h"
#include "PPRenderPass.h"

#include <d3d11.h>

#include "imgui/imgui.h"
#include "ResourceManagers/States/BlendStates.h"
#include "ResourceManagers/States/DSStates.h"
#include "ResourceManagers/States/RasterizerStates.h"
#include "ResourceManagers/States/Samplers.h"

using namespace Renderer;

const Renderer::ShaderDefines PPRenderPassDefines[] = {
    Renderer::ShaderDefines("RED"),
    Renderer::ShaderDefines("ALPHA"),
    Renderer::ShaderDefines("SINGLETEXTURE"),
};

PPRenderPass::PPRenderPassProvider::PPRenderPassProvider() {}

void PPRenderPass::PPRenderPassProvider::PatchPipelineState(Renderer::Pipeline* refToPS, uint32_t definesFlags)
{
    refToPS->bs = &BlendStates::NoAlpha;

    if (definesFlags & (PPALPHA))
    {
        refToPS->bs = &BlendStates::Alpha;
    }

    refToPS->dss = &DepthStencilStates::NoDSS;
    refToPS->rs = &RasterizerStates::All;
}
const D3D11_INPUT_ELEMENT_DESC DefaultInputElements[] =
{
    {"Position",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
    {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
};


Renderer::InputLayoutDescription PPRenderPass::PPRenderPassProvider::GetInputLayoutDescription(size_t definesFlags)
{
    return Renderer::InputLayoutDescription{(void*)DefaultInputElements,std::size(DefaultInputElements)};
}

const char* PPRenderPass::PPRenderPassProvider::GetShaderName()
{
    return "PostProcess";
}

Renderer::PipelineFactoryDescription PPRenderPass::PPRenderPassProvider::GetFactoryDescription()
{
    return {
        PPRenderPassDefines,
        std::size(PPRenderPassDefines)
    };
}

PPRenderPass::PPRenderPass(BaseRenderSystem& in):
    BaseRenderPass({"PostProcessShader.hlsl",in}),
    quad(in.pRenderer)
{
    sampler.filter = TextureFilter::TEXTUREFILTER_POINT;
    sampler.addressU = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;
    sampler.addressV = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;
    sampler.addressW = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;

    diffuseSampler.filter = TextureFilter::TEXTUREFILTER_ANISOTROPIC;
    diffuseSampler.addressU = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;
    diffuseSampler.addressV = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;
    diffuseSampler.addressW = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;
    diffuseSampler.maxAnisotropy = 16;

    pConstBuffer = renderDevice->CreateConstBuffer(sizeof(localData));
    localData.lightAdd = 0.3;
}


void PPRenderPass::Init(const char* dirr)
{
    BaseRenderPass::Init(dirr, new PPRenderPassProvider());

    texturesList = baseRendererParams.renderSystem.texturesManger->GetRenderTargetsList();
    texutersFlags.assign(texturesList.size(), 0);
}


void PPRenderPass::Render()
{
    quad.Render();
    RenderIMGUI();

    RenderTargetBinding* targets[] = {
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("preAAcolor"))
    };

    uint64_t flags = 0;

    renderDevice->SetRenderTargets(targets, std::size(targets), nullptr);

    renderDevice->VerifyPixelSampler(0, Samplers::pointClamp);
    renderDevice->VerifyPixelSampler(1, Samplers::pointClamp);

    if (ShowDebugTexture)
    {
        flags |= PPSINGLETEXTURE;
        renderDevice->VerifyPixelTexture(0, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SIDRT("diffuseColor"))->texture);
        for (int i = 0; i < texutersFlags.size(); ++i)
        {
            if (texutersFlags[i])
            {
                renderDevice->VerifyPixelTexture(0, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SIDRT(texturesList[i]))->texture);
            }
        }
    }
    else
    {
        renderDevice->VerifyPixelTexture(0, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("diffuseColor				"))->texture);
        renderDevice->VerifyPixelTexture(1, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("bloomMask					"))->texture);
        renderDevice->VerifyPixelTexture(2, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("lightColor				"))->texture);
        renderDevice->VerifyPixelTexture(3, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("alphaSurfaces				"))->texture);
        renderDevice->VerifyPixelTexture(4, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("oclusionField				"))->texture);
    }


    renderDevice->VerifyConstBuffer(pConstBuffer, ppCosntBuffer.slot);
    renderDevice->SetConstBuffer(pConstBuffer, &localData);

    renderDevice->ApplyPipelineState(factory->GetState(PPZERO | flags));
    renderDevice->DrawIndexedPrimitives(Renderer::PrimitiveType::PRIMITIVETYPE_TRIANGLESTRIP, 0, 0, 0, 0, 2);
    renderDevice->VerifyPixelTexture(0, nullptr);
    renderDevice->VerifyPixelTexture(1, nullptr);
    renderDevice->VerifyPixelTexture(2, nullptr);
    renderDevice->VerifyPixelTexture(3, nullptr);
    renderDevice->VerifyPixelTexture(4, nullptr);
    renderDevice->VerifyPixelTexture(8, nullptr);
    renderDevice->VerifyPixelTexture(9, nullptr);

    if (!ShowDebugTexture)
    {
        renderDevice->VerifyConstBuffer(pConstBuffer, ppCosntBuffer.slot);
        renderDevice->VerifyPixelSampler(0, Samplers::pointClamp);

        renderDevice->SetRenderTargets(targets, 1, nullptr);

        renderDevice->VerifyPixelTexture(5, nullptr);
        renderDevice->VerifyPixelTexture(6, nullptr);

        PipelineFactoryFlags f;
        f.pipelineFlags |= PPALPHA;
        renderDevice->ApplyPipelineState(factory->GetState(f));
        renderDevice->DrawIndexedPrimitives(Renderer::PrimitiveType::PRIMITIVETYPE_TRIANGLESTRIP, 0, 0, 0, 0, 2);
        

        renderDevice->VerifyPixelTexture(3, nullptr);
    }
}

void PPRenderPass::PreRender()
{
}

void PPRenderPass::PostRender()
{
}


void PPRenderPass::RenderIMGUI()
{
    ImGui::Begin("PostProcess settings.");                          // Create a window called "Hello, world!" and append into it.



    ImGui::Checkbox("ShowDebugTexture", &ShowDebugTexture);

    for (int i = 0; i < texturesList.size(); ++i)
    {
        ImGui::Checkbox(texturesList[i], (bool*)(texutersFlags.data() + i));
    }

    //ImGui::Checkbox("LightOnly", &lightOnly);
    //ImGui::Checkbox("BloomOnly", &bloomOnly);
    //ImGui::Checkbox("AlphaOnly", &alphaOnly);
    //ImGui::Checkbox("BlureOnly", &blureOnly);
    //ImGui::Checkbox("OcclusionOnly", &occlusionOnly);
    //ImGui::Checkbox("NormalsOnly", &normalsOnly);
    ////ImGui::SliderInt("MotionBlure samples", &localData.numSampes, 1,10);
    //ImGui::SliderFloat("light Add", &localData.lightAdd, 0, 1.0);
    ImGui::End();
}



PPRenderPass::~PPRenderPass()
{
    delete factory;
}
