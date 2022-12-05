#include "LightRenderPass.h"

#include "RenderFlags.h"
#include "ResourceManagers/States/Samplers.h"
#include "ResourceManagers/States/RasterizerStates.h"
#include "ResourceManagers/States/DSStates.h"
#include "Utils/PBRFlags.h"

using namespace Renderer;

LightRenderPass::DrawCall::DrawCall(
    ModelsManager::SavedModel model,
    const LightDrawData& data):
    model(model),
    data(data)
{}

PipelineFactoryFlags LightRenderPass::ParseFlags(const LightDrawData& flags)
{
    PipelineFactoryFlags flagsOut;
    flagsOut.definesFlags = 0;
    switch (flags.Light.LightType)
    {
    case LightTypes::Ambient:
    {
        flagsOut.definesFlags |= (uint)PixelFlagsLighting::AmbientLight;
        flagsOut.definesFlags |= (uint)PixelFlagsLighting::SCREEN_QUAD;
        break;
    }
    case LightTypes::Directional:
    {
        flagsOut.definesFlags |= (uint)PixelFlagsLighting::DirectionalLight;
        flagsOut.definesFlags |= (uint)PixelFlagsLighting::SCREEN_QUAD;
        break;
    }
    case LightTypes::Point:
    {
        flagsOut.definesFlags |= (uint)PixelFlagsLighting::PointLight;
        break;
    }
    case LightTypes::Spot:
    {
        flagsOut.definesFlags |= (uint)PixelFlagsLighting::SpotLight;
        break;
    }
        
    }

    //flagsOut.pipelineFlags = flags.flags;

    return flagsOut;
}


LightRenderPass::LightRenderPass(BaseRenderSystem& renderSystem) :
    GachiBasePass({"PBRLighting.hlsl",renderSystem}),
    quad(renderSystem.pRenderer)
{
    uint32_t width, height;
    renderDevice->GetMainViewportSize(width, height);
    pDataCB = renderDevice->CreateConstBuffer(sizeof(dataBuffer));

    vp.x = 0;
    vp.y = 0;
    vp.w = width;
    vp.h = height;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;


    renderSystem.texturesManger->CreatePublicRenderTarget({"Light",SurfaceFormat::SURFACEFORMAT_VECTOR4,false,true});
}

void LightRenderPass::Draw(const LightDrawData& drawData)
{
    switch (drawData.Light.LightType)
    {
    case LightTypes::Ambient:
    case LightTypes::Directional:
    {
        drawCalls.push_back(DrawCall(
            {},
            drawData));
        break;
    }
    }
}



void LightRenderPass::Init(const char* dirr)
{
    BaseRenderPass::Init(dirr, new LightPassProvider());
}

void LightRenderPass::PreRender()
{
    RenderTargetBinding* targets[] = {
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Light")),
    };

    renderDevice->SetRenderTargets(targets, std::size(targets), baseRendererParams.renderSystem.texturesManger->depthBuffer);
    renderDevice->Clear(CLEAROPTIONS_TARGET, {}, 0, 0);
    renderDevice->Clear(CLEAROPTIONS_DEPTHBUFFER, {}, 1, 0);
}

void LightRenderPass::Resize()
{
    uint32_t width, height;
    renderDevice->GetMainViewportSize(width, height);

    vp.w = width;
    vp.h = height;
}

void LightRenderPass::Render()
{
    RenderTargetBinding* targets[] = {
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Light")),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Bloom")),
    };

    renderDevice->SetRenderTargets(targets, std::size(targets), baseRendererParams.renderSystem.texturesManger->depthBuffer);


    size_t lastFlags = -1;

    renderDevice->SetConstBuffer(pDataCB, &dataBuffer);
    renderDevice->VerifyConstBuffer(pDataCB, lightCosntBuffer.slot);
    
    renderDevice->VerifyPixelTexture(0, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Diffuse"))->texture);
    renderDevice->VerifyPixelTexture(1, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Normals"))->texture);
    renderDevice->VerifyPixelTexture(2, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("MetRougAo"))->texture);
    renderDevice->VerifyPixelTexture(3, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Emissive"))->texture);
    renderDevice->VerifyPixelTexture(4, baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("WorldPosition"))->texture);


    renderDevice->VerifyPixelSampler(0, Samplers::anisotropic16);


    for (size_t i = 0; i < drawCalls.size(); i++)
    {
        const auto& data = drawCalls[i].data;
        PipelineFactoryFlags flags = ParseFlags(data);
        if (flags.flags != lastFlags)
        {
            renderDevice->ApplyPipelineState(factory->GetState(flags));
            lastFlags = flags.flags;
        }
        
        auto& light = data.Light;

        
        auto lightDataFrom = light.GetLightData();
        
      
        dataBuffer.Light.Color  = lightDataFrom.Color ;
        dataBuffer.Light.Dir    = lightDataFrom.Dir   ;
        dataBuffer.Light.Params = lightDataFrom.Params;
        dataBuffer.Light.Pos    = lightDataFrom.Pos   ;

        renderDevice->SetConstBuffer(pDataCB, &dataBuffer);
        
        if (light.LightType == LightTypes::Ambient || light.LightType == LightTypes::Directional)
        {
            renderDevice->ApplyRasterizerState(RasterizerStates::ClockWise);
            renderDevice->SetDepthStencilState(DepthStencilStates::DSS);

            //context->VSSetShader(LightingVSs[VertexFlagsLighting::SCREEN_QUAD], nullptr, 0);
            //context->PSSetShader(LightingPSs[light->GetLightFlags()], nullptr, 0);

            //context->IASetInputLayout(nullptr);
            //context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

            //context.InputAssembler.SetVertexBuffers
            //context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

            quad.Render();
            
            renderDevice->DrawIndexedPrimitives(PrimitiveType::PRIMITIVETYPE_TRIANGLELIST,
                0,0,6,0,2);

        }
        else {
            //context->RSSetState(rastCullFront);
            //context->OMSetDepthStencilState(dsLightingGreater, 0);
//
            //context->VSSetShader(LightingVSs[VertexFlagsLighting::NONE], nullptr, 0);
            //context->PSSetShader(LightingPSs[light->GetLightFlags()], nullptr, 0);
//
            //context->IASetInputLayout(layoutLighting);
            //context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
            //unsigned int offset = 0;
            //context->IASetVertexBuffers(0, 1, &light->MeshRep->VertexBuffer, &light->MeshRep->Stride, &offset);
            //context->IASetIndexBuffer(light->MeshRep->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
//
            //context->DrawIndexed(light->MeshRep->IndexCount, 0, 0);
        }

      // renderDevice->ApplyVertexBufferBinding(drawCalls[i].model.vertexBuffer);
      // renderDevice->ApplyIndexBufferBinding(drawCalls[i].model.indexBuffer, drawCalls[i].model.indexBufferElementSize);
      //
      //
      // dataBuffer.world = drawCalls[i].data.world.GetTransform();
      //
      // //if (drawCalls[i].data.isGun)
      // //	dataBuffer.blurSwitch = 1.f;
      // renderDevice->SetConstBuffer(pDataCB, &dataBuffer);
      //
      // renderDevice->DrawIndexedPrimitives(
      //     drawCalls[i].model.pt, 0, 0, 0, 0,
      //     drawCalls[i].model.primitiveCount);
    }
}

void LightRenderPass::PostRender()
{
    drawCalls.clear();
}

LightRenderPass::~LightRenderPass()
{
    renderDevice->AddDisposeConstBuffer(pDataCB);
}

void LightRenderPass::SetupSettings(const RenderSettings& Settings)
{
    Init(Settings.shadersDirr);
}
