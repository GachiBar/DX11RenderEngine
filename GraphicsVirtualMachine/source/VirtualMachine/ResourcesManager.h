#pragma once
#include <unordered_map>
#include <type_traits>

#include "GPUResourcesDescription/GPUInputLayout.h"
#include "GPUResourcesDescription/GPUResource.h"
#include "GPUResourcesDescription/GPUShader.h"


namespace GVM {

class VirtualMachine;
struct PipelineSnapshot;

class ResourcesManager {

    uintptr_t freeIndex = 1;
    std::unordered_map<uintptr_t, GpuResource> Resources = {{0,{}}};
    std::unordered_map<uintptr_t, GpuResourceView> ResourceViews = {{0,{}}};

    uint32_t dataSize = 0;
    std::vector<uint8_t> savedData;

    std::vector<Resource*> ResourcesRequiredUpdate;
    std::vector<ResourceView*> ResourceViewsRequiredUpdate;

    IRenderDevice* device;

    static uint32_t CalculateResourceSize(const BufferResourceDesc& desc);
    static uint32_t CalculateResourceSize(const ResourceDesc& desc);
    static uint32_t CalculateResourceSize(const Texture1DResourceDesc& desc);
    static uint32_t CalculateResourceSize(const Texture2DResourceDesc& desc);
    static uint32_t CalculateResourceSize(const Texture3DResourceDesc& desc);
    static uint32_t CalculateResourceSize(const TextureCubeResourceDesc& desc);

    friend VirtualMachine;
    friend PipelineSnapshot;


    ResourcesManager() = default;

#pragma region Resources
    GpuResource& CreateResource(const ResourceDesc& desc);
    GpuResource& CreateBuffer(const BufferResourceDesc& desc);
    GpuResource& CreateTexture1D(const Texture1DResourceDesc& desc);
    GpuResource& CreateTexture2D(const Texture2DResourceDesc& desc);
    GpuResource& CreateTexture3D(const Texture3DResourceDesc& desc);
    GpuResource& CreateTextureCube(const TextureCubeResourceDesc& desc);
    GpuResource& CreateVertexBuffer(const BufferResourceDesc& desc);
    GpuResource& CreateConstBuffer(const BufferResourceDesc& desc);
    GpuResource& CreateIndexBuffer(const BufferResourceDesc& desc);

    GpuResourceView& CreateConstBufferView(const ConstBufferViewDesc& desc);
    GpuResourceView& CreateVertexBufferView(const VertexBufferViewDesc& desc);
    GpuResourceView& CreateIndexBufferView(const IndexBufferViewDesc& desc);
    GpuResourceView& CreateDepthStencilView(const DepthStencilViewDesc& desc);
    GpuResourceView& CreateShaderResourceView(const ShaderResourceViewDesc& desc);
    GpuResourceView& CreateRenderTargetView(const RenderTargetViewDesc& desc);
    GpuResourceView& CreateDepthStencilsView(const DepthStencilViewDesc& description);
    GpuResourceView& CreateUATargetView(const UATargetViewDesc& desc);
#pragma endregion



#pragma region Shaders

    std::vector<GPUShader> SavedShaders = {GPUShader(ShaderDesc(), 0)};
    std::vector<GPUInputLayout> SavedInputLayouts = {GPUInputLayout({},0)};

    GPUShader& CreateShader(const ShaderDesc& desc);
    GPUInputLayout& CreateInputLayout(const InputAssemblerDeclarationDesc& desc);

#pragma endregion

    //void UpdateResources();
    
    GpuResource& GetResource(const Resource* resource);
    GpuResourceView& GetResourceView(const ResourceView* resourceView);

public:
    IRenderDevice::RESOURCEHANDLE GetRealResource(const Resource* resource);
    IRenderDevice::RESOURCEVIEWHANDLE GetRealResourceView(const ResourceView* resourceView);
    


    IRenderDevice::SHADERHANDLE GetRealShader(const Shader* shader);
    IRenderDevice::INPUTLAYOUTHANDLE GetRealInputLayout(const InputLayout* inputLayout);
    
    void AddDisposeResource(const Resource* resource);
    void AddDisposeResourceView(const ResourceView* resourceView);
};

}
