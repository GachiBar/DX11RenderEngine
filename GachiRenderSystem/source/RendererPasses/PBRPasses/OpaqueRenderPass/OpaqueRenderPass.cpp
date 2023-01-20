#include "OpaqueRenderPass.h"

#include "RenderFlags.h"
#include "ResourceManagers/States/Samplers.h"
#include "Utils/PBRFlags.h"

using namespace Renderer;

OpaqueRenderPass::DrawCall::DrawCall(
        ModelsManager::SavedModel model,
            
        TexturesManager::TextureCache diffuse,
        TexturesManager::Float3TextureCache normal,
        TexturesManager::FloatTextureCache roughness,
        TexturesManager::FloatTextureCache metallic,
        
        const OpaqueModelDrawData& data):
    model(model),
    diffuse(diffuse),
    normal(normal),
    roughness(roughness),
    metallic(metallic),
    data(data) 
{}

PipelineFactoryFlags OpaqueRenderPass::ParseFlags(const OpaqueModelDrawData& flags)
{
    PipelineFactoryFlags flagsOut;
    flagsOut.definesFlags = 0;
    if (!flags.materialData.diffuseData.isTextured)
        flagsOut.definesFlags |= ODIFFUSE;
    if (!flags.materialData.metallicData.isTextured)
        flagsOut.definesFlags |= OMETALIC;
    if (!flags.materialData.normalData.isTextured)
        flagsOut.definesFlags |= ONORMAL;
    if (!flags.materialData.roughnessData.isTextured)
        flagsOut.definesFlags |= OROUGHNESS;

    //flagsOut.pipelineFlags = flags.flags;

    return flagsOut;
}


OpaqueRenderPass::OpaqueRenderPass(BaseRenderSystem& renderSystem) : GachiBasePass({"PBRFillGBuffer.hlsl",renderSystem})
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


    renderSystem.texturesManger->CreatePublicRenderTarget({"Diffuse"        , SurfaceFormat::SURFACEFORMAT_COLOR, false, true});
    renderSystem.texturesManger->CreatePublicRenderTarget({"Emissive"       , SurfaceFormat::SURFACEFORMAT_COLOR, false, true});
    renderSystem.texturesManger->CreatePublicRenderTarget({"Normals"        , SurfaceFormat::SURFACEFORMAT_VECTOR4, false, true});
    renderSystem.texturesManger->CreatePublicRenderTarget({"MetRougAo"      , SurfaceFormat::SURFACEFORMAT_VECTOR4, false, true});
    renderSystem.texturesManger->CreatePublicRenderTarget({"WorldPosition"  , SurfaceFormat::SURFACEFORMAT_VECTOR4, false, true});
    renderSystem.texturesManger->CreatePublicRenderTarget({"AccumulationBuf", SurfaceFormat::SURFACEFORMAT_VECTOR4, false, true});
    renderSystem.texturesManger->CreatePublicRenderTarget({"Bloom"          , SurfaceFormat::SURFACEFORMAT_VECTOR4, false, true});
    renderSystem.texturesManger->CreatePublicRenderTarget({"Id"             , SurfaceFormat::SURFACEFORMAT_UINT, false, true});
}

void OpaqueRenderPass::Draw(const OpaqueModelDrawData& drawData)
{
    
    drawCalls.push_back(DrawCall(
            baseRendererParams.renderSystem.modelsManager->GetModel(drawData.modelId),
            baseRendererParams.renderSystem.texturesManger->GetImg(drawData.materialData.diffuseData.textureId),
            baseRendererParams.renderSystem.texturesManger->GetFloat3Img(drawData.materialData.normalData.textureId),
            baseRendererParams.renderSystem.texturesManger->GetFloatImg(drawData.materialData.roughnessData.textureId),
            baseRendererParams.renderSystem.texturesManger->GetFloatImg(drawData.materialData.metallicData.textureId),
            drawData)
    );
}



void OpaqueRenderPass::Init(const char* dirr)
{
    BaseRenderPass::Init(dirr, new OpaquePassProvider());
}

void OpaqueRenderPass::PreRender()
{
    RenderTargetBinding* targets[] = {
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Diffuse"         )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Emissive"        )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Normals"         )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("MetRougAo"       )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("WorldPosition"   )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("AccumulationBuf" )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Bloom"           )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Id"           )),
    };

    renderDevice->SetRenderTargets(targets, std::size(targets), baseRendererParams.renderSystem.texturesManger->depthBuffer);
    renderDevice->Clear(CLEAROPTIONS_TARGET, {}, 0, 0);
    renderDevice->Clear(CLEAROPTIONS_DEPTHBUFFER, {}, 1, 0);
}

void OpaqueRenderPass::Resize()
{
    
    uint32_t width, height;
    renderDevice->GetMainViewportSize(width, height);
    
    vp.w = width;
    vp.h = height;
}

void OpaqueRenderPass::Render()
{

    RenderTargetBinding* targets[] = {
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Diffuse"        )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Normals"         )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("MetRougAo"      )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Emissive"       )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("WorldPosition"  )),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Id"  )),
    };

    renderDevice->SetRenderTargets(targets, std::size(targets), baseRendererParams.renderSystem.texturesManger->depthBuffer);


    size_t lastFlags = -1;

    renderDevice->VerifyConstBuffer(pDataCB, opaqueCosntBuffer.slot);


    renderDevice->VerifyPixelSampler(0, Samplers::anisotropic16);


    for (size_t i = 0; i < drawCalls.size(); i++)
    {
        const auto& data = drawCalls[i].data;
        const auto& material = data.materialData;
        PipelineFactoryFlags flags = ParseFlags(data);
        if (flags.flags != lastFlags)
        {
            renderDevice->ApplyPipelineState(factory->GetState(flags));
            lastFlags = flags.flags;
        }

        renderDevice->ApplyVertexBufferBinding(drawCalls[i].model.vertexBuffer);
        renderDevice->ApplyIndexBufferBinding(drawCalls[i].model.indexBuffer, drawCalls[i].model.indexBufferElementSize);

        
        dataBuffer.world = drawCalls[i].data.world.GetTransform();
        dataBuffer.oldWorld = drawCalls[i].data.oldWorld.GetTransform();
        
        dataBuffer.diffuse = material.diffuseData.color.ToFloat4();
        dataBuffer.metallic = material.metallicData.metallic;
        dataBuffer.roughness = material.roughnessData.roughness;
        dataBuffer.MetallicMult = drawCalls[i].data.MetallicMult;
        dataBuffer.RoughnessMult = drawCalls[i].data.RoughnessMult;
        dataBuffer.normal = material.normalData.normal;
        dataBuffer.id = i+1;
        
        //if (drawCalls[i].data.isGun)
        //	dataBuffer.blurSwitch = 1.f;
        renderDevice->SetConstBuffer(pDataCB, &dataBuffer);
        renderDevice->VerifyPixelTexture(0, drawCalls[i].diffuse.texture);
        renderDevice->VerifyPixelTexture(1, drawCalls[i].normal.texture);
        renderDevice->VerifyPixelTexture(2, drawCalls[i].roughness.texture);
        renderDevice->VerifyPixelTexture(3, drawCalls[i].metallic.texture);

        renderDevice->DrawIndexedPrimitives(
            drawCalls[i].model.pt, 0, 0, 0, 0,
            drawCalls[i].model.primitiveCount);
    }

    renderDevice->GetDataFrom(baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("Id"  )), idData);
}

void OpaqueRenderPass::PostRender()
{
    drawCalls.clear();
}

OpaqueRenderPass::~OpaqueRenderPass()
{
    renderDevice->AddDisposeConstBuffer(pDataCB);
}

void OpaqueRenderPass::SetupSettings(const RenderSettings& Settings)
{
    Init(Settings.shadersDirr);
}
