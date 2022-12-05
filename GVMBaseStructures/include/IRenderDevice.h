#pragma once
#include "GVMBaseStructures.h"
#include "GVMBaseStructures/RenderDeviceInitParams.h"


namespace GVM {

class VirtualMachine;
class ResourcesManager;
class GpuResource;
class GpuResourceView;

enum RenderDeviceLimitations {
    MAX_RENDERTARGET_ATTACHMENTS = 8,
    MAX_SAMPLER_ATTACHMENTS = 8,
    MAX_TEXTURE_ATTACHMENTS = 128,
    MAX_VIEWPORT_ATTACHMENTS = 8,
    MAX_CONSTBUFFER_ATTACHMENTS = 15,
};
#ifndef DX12
#define DX11
#endif

struct PipelineDescription;
class IRenderDevice {



    
public:
    IRenderDevice(const IRenderDevice&) = delete;
    IRenderDevice(const IRenderDevice&&) = delete;
    virtual ~IRenderDevice() = 0 {}
    virtual void GetBackbufferSize(uint32_t& w, uint32_t& h) = 0;


protected:

#ifdef DX11
    
protected:

    struct IPlaceable {
    public:
        //virtual void Place(void* ptr) const = 0;
        //virtual ~IPlaceable() = 0 {};
    };
    
    class IResource : public IPlaceable {};
    class IResourceView : public IPlaceable {};
    class IInputLayout : public IPlaceable {};
    class IShader : public IPlaceable {};

    class IVertexBufferView     : public IResourceView {};
    class IIndexBufferView      : public IResourceView {};
    class IConstBufferView      : public IResourceView {};
    class IRenderTargetView     : public IResourceView {};
    class IDepthStencilView     : public IResourceView {};
    class IShaderResourceView   : public IResourceView {};
    class IUATargetView         : public IResourceView {};
    
public: 
    typedef IResource*           RESOURCEHANDLE         ;
    typedef IResourceView*       RESOURCEVIEWHANDLE     ;
    typedef IInputLayout*        INPUTLAYOUTHANDLE      ;
    typedef IShader*             SHADERHANDLE           ;
        
    typedef IVertexBufferView*   VERTEXBUFFERVIEWHANDLE   ;
    typedef IIndexBufferView*    INDEXBUFFERVIEWHANDLE    ;
    typedef IConstBufferView*    CONSTBUFFERVIEWHANDLE    ;
    typedef IRenderTargetView*   RENDERTARGETVIEWHANDLE   ;
    typedef IDepthStencilView*   DEPTHSTENCILVIEWHANDLE   ;
    typedef IShaderResourceView* SHADERRESOURCEVIEWHANDLE ;
    typedef IUATargetView*       UATARGETVIEWHANDLE       ;

#elif defined(DX12)
    
    struct IPlaceable {
    public:
        //virtual void Place(void* ptr) const = 0;
        //virtual ~IPlaceable() = 0 {};
    };
    class IResource : public IPlaceable {};
    class IInputLayout : public IPlaceable {};
    class IShader : public IPlaceable {};
    
    struct DX12ResourceView {
        uint64_t data;
    };
    struct DX1VertexBufferView   :  DX12ResourceView{};
    struct DX1IndexBufferView    :  DX12ResourceView{};
    struct DX1ConstBufferView    :  DX12ResourceView{};
    struct DX1RenderTargetView   :  DX12ResourceView{};
    struct DX1DepthStencilView   :  DX12ResourceView{};
    struct DX1ShaderResourceView :  DX12ResourceView{};
    struct DX1UATargetView       :  DX12ResourceView{};

#endif

    
    //friend class VirtualMachine;
    //friend class ResourcesManager;
    //friend class GpuResource;
    //friend class GpuResourceView;
    

    IRenderDevice(const RenderDeviceInitParams& initParams, bool debugMode = true) {}

    virtual void Draw(const DrawCall& call) = 0;
    virtual void Present() = 0;


    virtual RESOURCEHANDLE CreateResource(const GpuResource& resource) = 0;
    virtual void DestroyResource(const RESOURCEHANDLE resource) = 0;
    virtual void DestroyResourceView(const RESOURCEVIEWHANDLE resource) = 0;
    virtual RESOURCEVIEWHANDLE CreateResourceView(const GpuResourceView& desc, const GpuResource& ResourceDesc) = 0;



    virtual SHADERHANDLE CreateShader(const ShaderDesc& desc) = 0;
    virtual INPUTLAYOUTHANDLE CreateInputLayout(const InputAssemblerDeclarationDesc& desc, const ShaderDesc& Shader) = 0;
    virtual void ClearState() = 0;


    virtual void BeginEvent(const char* name) = 0;
    virtual void EndEvent() = 0;

    
    virtual void* GetNativeTexture(RESOURCEVIEWHANDLE view) = 0;

    virtual void SetResourceData(
        const GpuResource& resource,
        uint16_t dstSubresource,
        const UBox& rect,
        const void* pSrcData,
        int32_t srcRowPitch = 0,
        int32_t srcDepthPitch = 0
    ) = 0;


    virtual void SyncBlockExecutionStart() = 0;
    virtual void SyncResourcesRead(RESOURCEHANDLE data[], size_t size) = 0;
    virtual void SyncResourcesWrite(RESOURCEHANDLE data[], size_t size) = 0;
    virtual void SyncBlockExecutionEnd() = 0;

#pragma region SetupPipeline

    

    virtual void SetupPipeline(const PipelineDescription& Pipeline) = 0;

    virtual void SetupVertexBuffers(const VERTEXBUFFERVIEWHANDLE vertexBuffers[], uint8_t num) = 0;
    virtual void SetupIndexBuffers(const INDEXBUFFERVIEWHANDLE indices) = 0;
    virtual void SetupTextures(RESOURCEVIEWHANDLE textures[], uint8_t num) = 0;
    virtual void SetupRenderTargets(const RENDERTARGETVIEWHANDLE renderTargets[], int32_t num, DEPTHSTENCILVIEWHANDLE depthStencilBuffer) = 0;
    virtual void SetupUATargets(UATARGETVIEWHANDLE ua_targets[], uint8_t uint8) = 0;
    virtual void SetupConstBuffers(CONSTBUFFERVIEWHANDLE constBuffers[], uint8_t num) = 0;


#pragma endregion

    virtual void ClearRenderTarget(RENDERTARGETVIEWHANDLE rtView, FColor color) = 0;
    virtual void ClearDepthStencil(DEPTHSTENCILVIEWHANDLE dsView, float depth, int8_t stencil) = 0;

    virtual void ResizeBackbuffer(int32_t width, int32_t height) = 0;

};


struct PipelineDescription
{
    Compressed::ViewportDesc* viewports; uint8_t viewportsNum;
    Compressed::CoreBlendDesc blendState;
    Compressed::DepthStencilStateDesc depthStencilState;
    Compressed::RasterizerStateDesc rasterizerState;
    const Compressed::SamplerStateDesc* samplers; uint8_t samplersNum;
    EPrimitiveTopology topology;

    bool isCS = false;
    IRenderDevice::SHADERHANDLE VS;
    IRenderDevice::SHADERHANDLE PS;
    IRenderDevice::SHADERHANDLE GS;
    IRenderDevice::SHADERHANDLE HS;
    IRenderDevice::SHADERHANDLE DS;
    IRenderDevice::SHADERHANDLE CS;
    IRenderDevice::INPUTLAYOUTHANDLE layout;

    //void SetupViewports(const Compressed::ViewportDesc viewport[], uint8_t num);
    //void SetupBlendState(const Compressed::CoreBlendDesc& blendState);
    //void SetupDepthStencilState(const Compressed::DepthStencilStateDesc& depthStencilState);
    //void SetupRasterizerState(const Compressed::RasterizerStateDesc& rasterizerState);
    //void SetupSamplers(const Compressed::SamplerStateDesc samplers[], uint8_t num);
    //void SetupPrimitiveTopology(const EPrimitiveTopology topology);
    //void SetupInputLayout(INPUTLAYOUTHANDLE layout);
    //void SetupShader(SHADERHANDLE shader, EShaderType type);
};
    
}