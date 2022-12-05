#include "GraphicsAPI.h"
#include "VirtualMachine/VirtualMachine.h"
#include "VirtualMachine/PipelineSnapshot.h"

using namespace GVM;

GraphicsApi::GraphicsApi(IRenderDevice* RenderDevice):
    ps(new PipelineSnapshot()), graphicsMachine(new VirtualMachine(RenderDevice)),
    rd(RenderDevice)
{
    
}

GraphicsApi::~GraphicsApi()
{
    delete ps;
    delete graphicsMachine;
}

void GraphicsApi::RunVM()
{
    graphicsMachine->RunVM();
}

void GraphicsApi::Present()
{
    graphicsMachine->Present();
}


void GraphicsApi::ClearRenderTargets(RenderTargetView* renderTargets[], int32_t numRenderTargets, FColor color)
{
    for (int i = 0; i < numRenderTargets; i++)
    {
        graphicsMachine->ClearRenderTarget(renderTargets[i], color);
    }
    
}
void GraphicsApi::ClearRenderTarget(RenderTargetView* renderTarget, FColor color)
{
    graphicsMachine->ClearRenderTarget(renderTarget, color);
}
void GraphicsApi::ClearDepthStencil(DepthStencilView* septhStencil, float depth, int8_t stencil)
{
    graphicsMachine->ClearDepthStencil(septhStencil, depth, stencil);
}

void GraphicsApi::ClearState()
{
    //ps->VS = ps->PS = ps->CS = ps->GS = ps->HS = ps->DS = nullptr;
    //
    //ps->DrawCallsNum = 0;
    //ps->primitiveType = EPrimitiveTopology::PRIMITIVE_TOPOLOGY_UNDEFINED;
    //ps->rasterizerState = RasterizerStateDesc::Default;
    //ps->depthStencilState  = DepthStencilStateDesc::Default;
    //ps->vertexDeclaration = nullptr;
    //ps->DepthStencilBuffer = nullptr;
    //ps->mesh = Mesh::VoidMesh;
    //ps->blendDesc = CoreBlendDesc();
//
    //ps->renderTargetsNum = 0;
    //ps->RenderTargets = {};
    //ps->samplersNum = 0;
    //ps->Samplers = {};
    //ps->viewportsNum = 0;
    //ps->Viewports = {};
    //ps->constBuffersNum = 0;
    //ps->ConstBuffers = {};
    //ps->texturesNum = 0;
    //ps->Textures = {};
    *ps = PipelineSnapshot();
    graphicsMachine->ClearState();
    wasPSUpdated = true;
}




void GraphicsApi::DrawIndexedPrimitives( int32_t primitiveCount, int32_t startIndex, int32_t baseVertex)
{
    if (wasPSUpdated)
    {
        graphicsMachine->PushPSC(*ps);
        wasPSUpdated = false;
    }
    graphicsMachine->PushDrawCall(DrawCall{EDrawCallType::DRAW_INDEXED,{primitiveCount,startIndex,baseVertex,0,0}});
}

void GraphicsApi::DrawInstancedPrimitives(int32_t baseVertex, int32_t minVertexIndex, int32_t numVertices, int32_t startIndex, int32_t primitiveCount, int32_t instanceCount)
{
    if (wasPSUpdated)
    {
        graphicsMachine->PushPSC(*ps);
        wasPSUpdated = false;
    }
    graphicsMachine->PushDrawCall(DrawCall{EDrawCallType::DRAW_INDEXED_INSTANCED,{baseVertex,minVertexIndex,numVertices,startIndex,primitiveCount}});
}

void GraphicsApi::DrawPrimitives(int32_t vertexStart, int32_t primitiveCount)
{
    if (wasPSUpdated)
    {
        graphicsMachine->PushPSC(*ps);
        wasPSUpdated = false;
    }
    graphicsMachine->PushDrawCall(DrawCall{EDrawCallType::DRAW,{vertexStart,primitiveCount,0,0,0}});
}

void GraphicsApi::Dispatch(size_t x, size_t y, size_t z)
{
    if (wasPSUpdated)
    {
        graphicsMachine->PushPSC(*ps);
        wasPSUpdated = false;
    }
    graphicsMachine->PushDrawCall(DrawCall{EDrawCallType::DISPATCH,{x,y,z,0,0}});
}






void GraphicsApi::SetupViewports(const ViewportDesc viewport[], uint8_t num, uint8_t offset)
{
    wasPSUpdated = true;
    for (int i = 0; i < num; i++)
    {
        ps->Viewports[offset + i] = viewport[i];
    }
    ps->viewportsNum = std::max<uint8_t>(ps->viewportsNum, offset + num);
}
void GraphicsApi::SetupViewport(const ViewportDesc& viewport, uint8_t slot)
{
    wasPSUpdated = true;
    ps->Viewports[slot] = viewport;
    ps->viewportsNum = std::max<uint8_t>(ps->viewportsNum, slot);
}
void GraphicsApi::SetupCoreBlendState(const CoreBlendDesc& blendState)
{
    wasPSUpdated = true;
    ps->blendDesc = blendState;
}
void GraphicsApi::SetupBlendState(const BlendStateDesc& blendState, int slot)
{
    ps->RenderTargets[slot].BlendState = blendState;
}
void GraphicsApi::SetupCoreBlendFactor(const float blendFactor[4])
{
    ps->blendDesc.BlendFactor[0] = blendFactor[0];
    ps->blendDesc.BlendFactor[1] = blendFactor[1];
    ps->blendDesc.BlendFactor[2] = blendFactor[2];
    ps->blendDesc.BlendFactor[3] = blendFactor[3];
}
void GraphicsApi::SetupDepthStencilState(const DepthStencilStateDesc& depthStencilState)
{
    wasPSUpdated = true;
    ps->depthStencilState = depthStencilState;
}
void GraphicsApi::SetupRasterizerState(const RasterizerStateDesc& rasterizerState)
{
    wasPSUpdated = true;
    ps->rasterizerState = rasterizerState;
}
void GraphicsApi::SetupSamplers(const SamplerStateDesc samplers[], uint8_t num, uint8_t offset)
{
    wasPSUpdated = true;
    for (int i = 0; i < num; i++)
    {
        ps->Samplers[offset + i] = samplers[i];
    }
    ps->samplersNum = std::max<uint8_t>(ps->samplersNum, offset + num);
}

void GraphicsApi::SetupSampler(const SamplerStateDesc& sampler, uint8_t slot)
{
    wasPSUpdated = true;
    ps->Samplers[slot] = sampler;
    ps->samplersNum = std::max<uint8_t>(ps->samplersNum, slot+1);
}
void GraphicsApi::SetupPrimitiveTopology(const EPrimitiveTopology topology)
{
    wasPSUpdated = true;
    ps->primitiveTopology = topology;
}
void GraphicsApi::SetupVertexBuffer(const VertexBufferBinding& bindings)
{
    wasPSUpdated = true;
    ps->mesh.vertexBuffer = bindings;
}
void GraphicsApi::SetupIndexBuffer(IndexBufferView* indices)
{
    wasPSUpdated = true;
    ps->mesh.indexBuffer = (IndexBufferView*)indices;
}
void GraphicsApi::SetupMeshBuffers(const Mesh& bindings)
{
    ps->mesh = bindings;
}
void GraphicsApi::SetupTextures(ResourceView* textures[], uint8_t num, uint8_t offset)
{
    wasPSUpdated = true;
    for (int i = 0; i < num; i++)
    {
        ps->Textures[i + offset] = (ResourceView*)textures[i];
    }
    ps->texturesNum = std::max<uint8_t>(ps->texturesNum, offset + num);
}

void GraphicsApi::SetupTexture(ResourceView* texture, uint8_t slot)
{
    wasPSUpdated = true;
    ps->Textures[slot] = (ResourceView*)texture;
    ps->texturesNum = std::max<uint8_t>(ps->texturesNum, slot+1);
}

void GraphicsApi::SetupNumRenderTargets(int32_t num)
{
    ps->renderTargetsNum = num;
}

void GraphicsApi::SetupRenderTargets(const RenderTargetDesc renderTargets[], int32_t num, uint8_t offset,
                                     DepthStencilView* depthStencilBuffer)
{
    wasPSUpdated = true;
    ps->DepthStencilBuffer = depthStencilBuffer;

    for (int i = 0; i < num; i++)
    {
        ps->RenderTargets[i + offset] = renderTargets[i];
    }
    ps->renderTargetsNum = std::max<uint8_t>(ps->renderTargetsNum, offset + num);
}
void GraphicsApi::SetupRenderTarget(const RenderTargetDesc& renderTarget, int32_t slot, DepthStencilView* depthStencilBuffer)
{
    wasPSUpdated = true;
    ps->RenderTargets[slot] = renderTarget;
    ps->renderTargetsNum = std::max<uint8_t>(ps->renderTargetsNum, slot+1);
}
void GraphicsApi::SetupRenderTargets(RenderTargetView* renderTargets[], int32_t num, uint8_t offset, DepthStencilView* depthStencilBuffer)
{
    
    wasPSUpdated = true;
    ps->DepthStencilBuffer = depthStencilBuffer;

    for (int i = 0; i < num; i++)
    {
        ps->RenderTargets[i + offset].rtv = renderTargets[i];
    }
    ps->renderTargetsNum = offset + num;
}
void GraphicsApi::SetupRenderTarget(RenderTargetView* renderTarget, int32_t slot, DepthStencilView* depthStencilBuffer)
{
    wasPSUpdated = true;
    ps->RenderTargets[slot].rtv = renderTarget;
    ps->renderTargetsNum = std::max<uint8_t>(ps->renderTargetsNum, slot+1);
}

void GraphicsApi::SetupUATexture(UATargetView* uaTarget, int32_t slot)
{
    wasPSUpdated = true;
    ps->UATargets[slot] = uaTarget;
    ps->uaTargetsNum = std::max<uint8_t>(ps->uaTargetsNum, slot+1);

}

void GraphicsApi::SetupDepthStencilBuffer(DepthStencilView* depthStencilBuffer)
{
    wasPSUpdated = true;
    ps->DepthStencilBuffer = depthStencilBuffer;
}

void GraphicsApi::SetupShader(Shader* shader, EShaderType type)
{
    wasPSUpdated = true;
    switch (type)
    {
    case EShaderType::COMPUTE_SHADER: ps->CS = (Shader*)shader;
        break;
    case EShaderType::DOMAIN_SHADER: ps->DS = (Shader*)shader;
        break;
    case EShaderType::GEOMETRY_SHADER: ps->GS = (Shader*)shader;
        break;
    case EShaderType::HULL_SHADER: ps->HS = (Shader*)shader;
        break;
    case EShaderType::PIXEL_SHADER: ps->PS = (Shader*)shader;
        break;
    case EShaderType::VERTEX_SHADER: ps->VS = (Shader*)shader;
        break;
    default: ;
    }
}
void GraphicsApi::SetupConstBuffers(ConstBufferView* constBuffers[], uint8_t num, uint8_t offset)
{
    wasPSUpdated = true;
    for (int i = 0; i < num; i++)
    {
        ps->ConstBuffers[offset + i] = constBuffers[i];
    }
    ps->constBuffersNum = std::max<uint8_t>(ps->constBuffersNum, offset + num);
}
void GraphicsApi::SetupConstBuffer(ConstBufferView* constBuffer, uint8_t slot)
{
    wasPSUpdated = true;
    ps->ConstBuffers[slot] = constBuffer;
    ps->constBuffersNum = std::max<uint8_t>(ps->constBuffersNum, slot+1);
}

void GraphicsApi::SetupInputLayout(InputLayout* layout)
{
    wasPSUpdated = true;
    ps->InputDeclaration = layout;
}

 Resource* GraphicsApi::CreateBuffer(const BufferResourceDesc& description)
{
    return graphicsMachine->CreateBuffer(description);
}

 Resource* GraphicsApi::CreateTexture(const ResourceDesc& description)
{
    return graphicsMachine->CreateResource(description);
}

 Resource* GraphicsApi::CreateTexture1D(const Texture1DResourceDesc& description)
{
    return graphicsMachine->CreateTexture1D(description);
}

 Resource* GraphicsApi::CreateTexture2D(const Texture2DResourceDesc& description)
{
    return graphicsMachine->CreateTexture2D(description);
}

 Resource* GraphicsApi::CreateTexture3D(const Texture3DResourceDesc& description)
{
    return graphicsMachine->CreateTexture3D(description);
}

 Resource* GraphicsApi::CreateTextureCube(const TextureCubeResourceDesc& description)
{
    return graphicsMachine->CreateTextureCube(description);
}

 ConstBuffer* GraphicsApi::CreateConstBuffer(const BufferResourceDesc& description)
{
    return graphicsMachine->CreateConstBuffer(description);
}

 VertexBuffer* GraphicsApi::CreateVertexBuffer(const BufferResourceDesc& description)
{
    return graphicsMachine->CreateVertexBuffer(description);
}

 IndexBuffer* GraphicsApi::CreateIndexBuffer(const BufferResourceDesc& description)
{
    return graphicsMachine->CreateIndexBuffer(description);
}

 ShaderResourceView* GraphicsApi::CreateShaderResourceView(const ShaderResourceViewDesc& description)
{
    return graphicsMachine->CreateShaderResourceView(description);
}

 RenderTargetView* GraphicsApi::CreateRtView(const RenderTargetViewDesc& description)
{
    return graphicsMachine->CreateRenderTargetView(description);
}
DepthStencilView* GraphicsApi::CreateDepthStencilsView(const DepthStencilViewDesc& description)
{
    return graphicsMachine->CreateDepthStencilsView(description);
}

UATargetView* GraphicsApi::CreateUaView(const UATargetViewDesc& description)
{
    return graphicsMachine->CreateUATargetView(description);
}

 VertexBufferView* GraphicsApi::CreateVertexBufferView(const VertexBufferViewDesc& description)
{
    return graphicsMachine->CreateVertexBufferView(description);
}

 IndexBufferView* GraphicsApi::CreateIndexBufferView(const IndexBufferViewDesc& description)
{
    return graphicsMachine->CreateIndexBufferView(description);
}

 ConstBufferView* GraphicsApi::CreateConstBufferView(const ConstBufferViewDesc& description)
{
    return graphicsMachine->CreateConstBufferView(description);
}

void GraphicsApi::AddDisposeResource(Resource* resource)
{
    graphicsMachine->AddDisposeResource(resource);
}

void GraphicsApi::AddDisposeResourceView(ResourceView* resourceView)
{
    graphicsMachine->AddDisposeResourceView(resourceView);
}

void GraphicsApi::SetResourceData(Resource* resource, uint16_t dstSubresource, const UBox& rect,
    const void* pSrcData, int32_t srcRowPitch, int32_t srcDepthPitch)
{
    graphicsMachine->SetResourceData(resource,dstSubresource, rect,
    pSrcData, srcRowPitch, srcDepthPitch);
}

void GraphicsApi::SetVertexBufferData(VertexBuffer* vertexBuffer, const void* pSrcData, uint32_t dataLength, uint32_t offset,
    int32_t srcRowPitch, int32_t srcDepthPitch)
{ 
    graphicsMachine->SetVertexBufferData(vertexBuffer, pSrcData, dataLength, offset
    , srcRowPitch, srcDepthPitch);
}

void GraphicsApi::SetIndexBufferData(IndexBuffer* buffer, const void* pSrcData, uint32_t dataLength, uint32_t offset,
    int32_t srcRowPitch, int32_t srcDepthPitch)
{
    graphicsMachine->SetIndexBufferData(buffer, pSrcData, dataLength, offset
      , srcRowPitch, srcDepthPitch);
}


void GraphicsApi::SetConstBufferData(ConstBuffer* constBuffer, const void* data, uint32_t dataSize, uint32_t offset)
{
    graphicsMachine->SetConstBufferData(constBuffer, data, dataSize, offset);
}

InputLayout* GraphicsApi::CreateInputLayout(const InputAssemblerDeclarationDesc& desc, const ShaderDesc& Shader)
{
    return graphicsMachine->CreateInputLayout(desc, Shader);
}

Shader* GraphicsApi::CreateShader(const ShaderDesc& desc)
{
    return graphicsMachine->CreateShader(desc);
}

void GraphicsApi::BeginEvent(const char* name)
{
    graphicsMachine->BeginEvent(name);
}

void GraphicsApi::EndEvent()
{
    graphicsMachine->EndEvent();
}

void GraphicsApi::SetMarker(const char* name)
{
    
}

void GraphicsApi::ResizeBackbuffer(int32_t width, int32_t height)
{
    graphicsMachine->ResizeBackbuffer(width, height);
}

void GraphicsApi::GetBackBufferSize(uint32_t& w, uint32_t& h)
{
    rd->GetBackbufferSize(w,h);
}

void* GraphicsApi::GetNativeTexture(const ShaderResourceView* shView)
{
   return  graphicsMachine->GetNativeTexture(shView);
}
void GraphicsApi::AddDisposeShader(Shader* shader)
{
    //todo
}
void GraphicsApi::AddDisposeIL(InputLayout* input_layout)
{
    //todo
}
