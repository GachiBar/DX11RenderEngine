#pragma once
#include "GVMBaseStructures.h"
#include "IRenderDevice.h"

namespace GVM {
class VirtualMachine;
struct PipelineSnapshot;

class GraphicsApi {

#pragma region Fields

    PipelineSnapshot* ps;
    bool wasPSUpdated = true;
    VirtualMachine* graphicsMachine;
    IRenderDevice* rd;

#pragma endregion

public:
    GraphicsApi(IRenderDevice* RenderDevice);
    GraphicsApi(const GraphicsApi&) = delete;
    GraphicsApi(const GraphicsApi&&) = delete;
    ~GraphicsApi();

    /* Presentation */

    void RunVM();
    void SetBB();
    void Present();

#pragma region Clear

    void ClearRenderTargets(RenderTargetView* renderTargets[], int32_t numRenderTargets, FColor color);
    void ClearRenderTarget(RenderTargetView* renderTarget, FColor color);
    void ClearDepthStencil(DepthStencilView* septhStencil, float depth, int8_t stencil);
    void ClearState();


#pragma endregion

#pragma region DrawDispatch

    void DrawIndexedPrimitives(
        int32_t primitiveCount,
        int32_t startIndex,
        int32_t baseVertex
    );

    void DrawInstancedPrimitives(
        int32_t baseVertex,
        int32_t minVertexIndex,
        int32_t numVertices,
        int32_t startIndex,
        int32_t primitiveCount,
        int32_t instanceCount
    );

    void DrawPrimitives(
        int32_t vertexStart,
        int32_t primitiveCount
    );

    void Dispatch(size_t x, size_t y, size_t z = 1);

#pragma endregion

#pragma region SetupPipeline
    void SetupViewports(const ViewportDesc viewport[], uint8_t num, uint8_t offset);
    void SetupViewport(const ViewportDesc& viewport, uint8_t slot);

    //void SetScissorRect(const ScissorRectDesc** scissors, uint8_t num);
    //void SetScissorRect(const ScissorRect** scissors, uint8_t num);
    void SetupCoreBlendState(const CoreBlendDesc& blendState);
    void SetupBlendState(const BlendStateDesc& blendState, int slot);
    void SetupCoreBlendFactor(const float blendFactor[4]);

    void SetupDepthStencilState(const DepthStencilStateDesc& depthStencilState);
    void SetupRasterizerState(const RasterizerStateDesc& rasterizerState);

    void SetupSamplers(const SamplerStateDesc samplers[], uint8_t num, uint8_t offset);
    void SetupSampler(const SamplerStateDesc& sampler, uint8_t slot);
    
    void SetupPrimitiveTopology(const EPrimitiveTopology topology);

    void SetupVertexBuffer(const VertexBufferBinding& bindings);
    void SetupIndexBuffer(IndexBufferView* indices);
    void SetupMeshBuffers(const Mesh& bindings);
    void SetupTextures(ResourceView* textures[], uint8_t num, uint8_t offset);
    void SetupTexture(ResourceView* texture, uint8_t slot);

    void SetupNumRenderTargets(int32_t num);
    void SetupRenderTargets(const RenderTargetDesc renderTargets[], int32_t num, uint8_t offset, DepthStencilView* depthStencilBuffer);
    void SetupRenderTarget(const RenderTargetDesc& renderTarget, int32_t slot, DepthStencilView* depthStencilBuffer);
    void SetupRenderTargets(RenderTargetView* renderTargets[], int32_t num, uint8_t offset, DepthStencilView* depthStencilBuffer);
    void SetupRenderTarget(RenderTargetView* renderTarget, int32_t slot, DepthStencilView* depthStencilBuffer);
    void SetupUATexture(UATargetView* uaTarget, int32_t slot);
    
    void SetupDepthStencilBuffer(DepthStencilView* depthStencilBuffer);
    void SetupShader(Shader* shader, EShaderType type);

    void SetupConstBuffers(ConstBufferView* constBuffers[], uint8_t num, uint8_t offset);
    void SetupConstBuffer(ConstBufferView* constBuffer, uint8_t slot);
    void SetupInputLayout(InputLayout* layout);


#pragma endregion

#pragma region CreateDestroyResizeResources
    /* Resources */

    Resource* CreateBuffer(const BufferResourceDesc& description);
    Resource* CreateTexture(const ResourceDesc& description);
    Resource* CreateTexture1D(const Texture1DResourceDesc& description);
    Resource* CreateTexture2D(const Texture2DResourceDesc& description);
    Resource* CreateTexture3D(const Texture3DResourceDesc& description);
    Resource* CreateTextureCube(const TextureCubeResourceDesc& description);

    ShaderResourceView* CreateShaderResourceView(const ShaderResourceViewDesc& description);
    RenderTargetView* CreateRtView(const RenderTargetViewDesc& description);
    DepthStencilView* CreateDepthStencilsView(const DepthStencilViewDesc& description);
    UATargetView* CreateUaView(const UATargetViewDesc& description);

    VertexBuffer* CreateVertexBuffer(const BufferResourceDesc& description);
    IndexBuffer* CreateIndexBuffer(const BufferResourceDesc& description);
    ConstBuffer* CreateConstBuffer(const BufferResourceDesc& description);

    VertexBufferView* CreateVertexBufferView(const VertexBufferViewDesc& description);
    IndexBufferView* CreateIndexBufferView(const IndexBufferViewDesc& description);
    ConstBufferView* CreateConstBufferView(const ConstBufferViewDesc& description);

    //Resource* ResizeBuffer   (const Resource* texture, const BufferDesc& description);
    //Resource* ResizeTexture1D(const Resource* texture, const Texture1DDesc& description);
    //Resource* ResizeTexture2D(const Resource* texture, const Texture2DDesc& description);
    //Resource* ResizeTexture3D(const Resource* texture, const Texture3DDesc& description);


    void AddDisposeResource(Resource* resource);
    void AddDisposeResourceView(ResourceView* resourceView);
    //void AddDisposeConstBuffer(const IConstBuffer* constBuffers);
    //void AddDisposeVertexBuffer(const IVertexBuffer* buffer);
    //void AddDisposeIndexBuffer(const IIndexBuffer* buffer);

#pragma endregion

#pragma region SetResourcesData

    void SetResourceData(
        Resource* resource,
        uint16_t dstSubresource,
        const UBox& rect,
        const void* pSrcData,
        int32_t srcRowPitch = 0,
        int32_t srcDepthPitch = 0
    );

    void SetVertexBufferData(
        VertexBuffer* vertexBuffer,
        const void* pSrcData,
        uint32_t dataLength,
        uint32_t offset = 0,
        int32_t srcRowPitch = 0,
        int32_t srcDepthPitch = 0
    );

    void SetIndexBufferData(
        IndexBuffer* buffer,
        const void* pSrcData,
        uint32_t dataLength,
        uint32_t offset = 0,
        int32_t srcRowPitch = 0,
        int32_t srcDepthPitch = 0
    );//todo offset
    
    
    void SetConstBufferData(
        ConstBuffer* constBuffer,
        const void* data,
        uint32_t dataSize,
        uint32_t offset = 0);

#pragma endregion

    InputLayout* CreateInputLayout(const InputAssemblerDeclarationDesc& desc, const ShaderDesc& Shader);
    Shader* CreateShader(const ShaderDesc& desc);

    //void Flush();

    void BeginEvent(const char* name);
    void EndEvent();
    void SetMarker(const char* name);
    void ResizeBackbuffer(int32_t width, int32_t height);
    void GetBackBufferSize(uint32_t& w, uint32_t& h);
    void* GetNativeTexture(const ShaderResourceView* shView);
    void AddDisposeShader(Shader* shader);
    void AddDisposeIL(InputLayout* input_layout);
    void GetDataFrom(ShaderResourceViewDesc resource, std::vector<std::vector<unsigned>>& dst);
};
}
