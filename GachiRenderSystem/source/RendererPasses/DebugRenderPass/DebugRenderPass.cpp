#include "DebugRenderPass.h"

#include "RenderFlags.h"
#include "ResourceManagers/States/Samplers.h"

using namespace Renderer;


PipelineFactoryFlags DebugRenderPass::ParseFlags(const DebugPassFlags& flags)
{
    PipelineFactoryFlags flagsOut;
    flagsOut.definesFlags = 0;

    if (flags.isRed)
        flagsOut.definesFlags |= DRED;
    if (flags.is3D)
        flagsOut.definesFlags |= D3D;
    if (flags.is2D)
        flagsOut.definesFlags |= D2D;

    flagsOut.pipelineFlags = flags.flags;

    return flagsOut;
}


DebugRenderPass::DebugRenderPass(BaseRenderSystem& renderSystem)
    : GachiBasePass({"DebugShader.hlsl", renderSystem}),
      vetexBuffer2D(renderSystem.pRenderer, 10, 10),
      vetexBuffer3D(renderSystem.pRenderer, 10, 10)
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
}

void DebugRenderPass::Draw(const DebugDraw3DData& drawData)
{
    auto drawCall = DrawCall3D{vetexBuffer3D.AddMesh(drawData.mesh), drawData.world, drawData.color, drawData.flags};
    drawCall.flags.is3D = 1;
    drawCall.flags.is2D = 0;
    drawCalls3D.push_back(drawCall);
}

void DebugRenderPass::Draw(const DebugDraw2DData& drawData)
{
    auto drawCall = DrawCall2D{vetexBuffer2D.AddMesh(drawData.mesh), drawData.color, drawData.flags};
    drawCall.flags.is2D = 1;
    drawCall.flags.is3D = 0;
    drawCalls2D.push_back(drawCall);
}

void DebugRenderPass::Resize()
{
    uint32_t width, height;
    renderDevice->GetMainViewportSize(width, height);
    pDataCB = renderDevice->CreateConstBuffer(sizeof(dataBuffer));

    vp.x = 0;
    vp.y = 0;
    vp.w = width;
    vp.h = height;
}


void DebugRenderPass::Init(const char* dirr)
{
    BaseRenderPass::Init(dirr, new DebugPassProvider());
}

void DebugRenderPass::PreRender()
{
    RenderTargetBinding* targets[5] = {
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("preAAcolor")),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("lightColor")),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("velocityField")),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("blurMask")),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("normalsField")),
    };

    renderDevice->SetRenderTargets(targets, std::size(targets),
                                   baseRendererParams.renderSystem.texturesManger->depthBuffer);
    renderDevice->Clear(CLEAROPTIONS_TARGET, {}, 0, 0);
    renderDevice->Clear(CLEAROPTIONS_DEPTHBUFFER, {}, 1, 0);
}

void DebugRenderPass::Render()
{
    uint32_t width, height;
    renderDevice->GetMainViewportSize(width, height);

    RenderTargetBinding* targets[] = {
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("preAAcolor")),
    };

    renderDevice->SetRenderTargets(targets, std::size(targets), nullptr);

    vetexBuffer2D.UpdateBuffers(true);
    vetexBuffer3D.UpdateBuffers(true);

    uint32_t lastFlags = -1;

    renderDevice->VerifyConstBuffer(pDataCB, debugCosntBuffer.slot);

    renderDevice->VerifyPixelSampler(0, Samplers::anisotropic16);
    renderDevice->VerifyPixelTexture(0, baseRendererParams.renderSystem.texturesManger->depthBuffer->texture);

    renderDevice->ApplyMeshBuffersBinding(vetexBuffer2D.mesh);
    auto ortho = matrix::CreateOrthographic(width, height, 1, 2);

    for (size_t i = 0; i < drawCalls2D.size(); i++)
    {
        if (drawCalls2D[i].flags.flags != lastFlags)
        {
            PipelineFactoryFlags flags = ParseFlags(drawCalls2D[i].flags);
            renderDevice->ApplyPipelineState(factory->GetState(flags));
            lastFlags = drawCalls2D[i].flags.flags;
        }


        dataBuffer.color = drawCalls2D[i].color;
        dataBuffer.world = ortho.Transpose();

        renderDevice->SetConstBuffer(pDataCB, &dataBuffer);

        renderDevice->DrawIndexedPrimitives(
        (PrimitiveType)drawCalls2D[i].hash.pt, 0, 0, 0, drawCalls2D[i].hash.indexOffset,
        drawCalls2D[i].hash.numElem);
    }

    renderDevice->ApplyMeshBuffersBinding(vetexBuffer3D.mesh);

    for (size_t i = 0; i < drawCalls3D.size(); i++)
    {
        if (drawCalls3D[i].flags.flags != lastFlags)
        {
            PipelineFactoryFlags flags = ParseFlags(drawCalls3D[i].flags);
            renderDevice->ApplyPipelineState(factory->GetState(flags));
            lastFlags = drawCalls3D[i].flags.flags;
        }


        dataBuffer.color = drawCalls3D[i].color;
        dataBuffer.world = drawCalls3D[i].world.GetTransform();

        renderDevice->SetConstBuffer(pDataCB, &dataBuffer);

        renderDevice->DrawIndexedPrimitives(
            (PrimitiveType)drawCalls3D[i].hash.pt, 0, 0, 0, drawCalls3D[i].hash.indexOffset,
            drawCalls3D[i].hash.numElem);
    }
}

void DebugRenderPass::PostRender()
{
    drawCalls3D.clear();
    drawCalls2D.clear();
    vetexBuffer2D.Flush();
    vetexBuffer3D.Flush();
}

DebugRenderPass::~DebugRenderPass()
{
    renderDevice->AddDisposeConstBuffer(pDataCB);
}

void DebugRenderPass::SetupSettings(const RenderSettings& Settings)
{
    Init(Settings.shadersDirr);
}
