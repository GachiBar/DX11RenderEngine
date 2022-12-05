#pragma once

#define GVM_MSW

#include "winHandler.h"

#define DX11

#include "IRenderDevice.h"
#include "GraphicsExceptions/DxgiInfoManager.h"


#ifndef  TESTDEBUG 
#define TESTDEBUG 1
#endif




namespace GVM {

struct Dx11PlatformHandle : public PlatformHandle {

#if defined(GVM_LINUX)
//xcb_connection_t*                   connection;
xcb_window_t                        window;
#elif defined(GVM_MSW)
//HINSTANCE                           hinstance;
HWND                                hwnd;
#endif

};

class RenderDeviceDX11 final : public IRenderDevice {
public:

    RenderDeviceDX11(const RenderDeviceInitParams& initParams, bool debugMode);
    RenderDeviceDX11(const IRenderDevice& render_device) = delete;
    RenderDeviceDX11(const IRenderDevice&& render_device) = delete;
    ~RenderDeviceDX11() override;



#pragma region Fields

#if _DEBUG | TESTDEBUG
    DxgiInfoManager infoManager;
#endif
    
    HRESULT hr;
    
    
    wrl::ComPtr<ID3D11Device> device;
    wrl::ComPtr<ID3D11DeviceContext> context;
    wrl::ComPtr<IDXGISwapChain> swapchain;
    bool CS = false;
    bool PS = false;
    bool VS = false;
    bool GS = false;
    bool HS = false;
    bool DS = false;
    IUnknown* CSPtr = nullptr;
    IUnknown* PSPtr = nullptr;
    IUnknown* VSPtr = nullptr;
    IUnknown* GSPtr = nullptr;
    IUnknown* HSPtr = nullptr;
    IUnknown* DSPtr = nullptr;
    ID3D11BlendState* blendStatePtr;
    FColor blendFactorHash;
    ID3D11DepthStencilState* depthStencilStatePtr;
    ID3D11RasterizerState* rasterizerStatePtr;
    EPrimitiveTopology topologyHash;
    ID3D11SamplerState* Samplers[16] = {};
    uint8_t samplersNum = 0;
    D3D11_VIEWPORT d3d11viewports[20];
    uint8_t d3d11viewportsNum = 0;
    uint8_t texturesNum = 0;
    IUnknown* indexBufferPtr = nullptr;
    uint8_t vertexBuffersNum = 0;
    int32_t renderTargetsNum = 0;
    ID3D11Buffer* constBuffersPtr[32];
    uint8_t constBuffersNum = 0;
    IInputLayout* layoutPtr;

private:
    wrl::ComPtr<ID3DUserDefinedAnnotation> perf;
    wrl::ComPtr<IDXGIAdapter> adapter;
    wrl::ComPtr<IDXGIFactory> factory;
    D3D_FEATURE_LEVEL featureLevel;

    /* Capabilities */
    uint8_t debugMode;
    //uint32_t supportsDxt1;
    //uint32_t supportsS3tc;
    //int32_t maxMultiSampleCount;
    //D3D_FEATURE_LEVEL featureLevel;

    /* Presentation */
    uint8_t syncInterval;

    //D3D11Backbuffer backbuffer;
    //uint8_t backbufferSizeChanged;
    //Rect prevSrcRect;
    //Rect prevDstRect;

    /* Blend State */
    wrl::ComPtr<ID3D11BlendState> blendState;
    FColor blendFactor;
    uint32_t multiSampleMask;

    /* Depth Stencil State */
    wrl::ComPtr<ID3D11DepthStencilState> depthStencilState;
    int32_t stencilRef;

    /* Rasterizer State */
    D3D11_VIEWPORT viewport[10];
    int maxViewportSlot;
    Rect scissorRect;
    wrl::ComPtr<ID3D11RasterizerState> rasterizerState;

    /* Textures */
    std::vector<ID3D11ShaderResourceView*> Textures;

    /* Input Assembly */
    EPrimitiveTopology topology;
    std::vector<ID3D11Buffer*> vertexBuffers;
    std::vector<uint32_t> vertexBufferOffsets;
    std::vector<uint32_t> vertexBufferStrides;
    //ResourceViewDX12* indexBuffer;
    //ResourceViewDX12* vertexBuffer;
    size_t indexElementSize;

    ///* Resource Caches */
    //PackedStateArray blendStateCache;
    //PackedStateArray depthStencilStateCache;
    //PackedStateArray rasterizerStateCache;
    //PackedStateArray samplerStateCache;
    //PackedVertexBufferBindingsArray inputLayoutCache;

    /* Render Targets */
    int32_t numRenderTargets;
    size_t backBufferWidth;
    size_t backBufferHeight;
    ID3D11RenderTargetView* swapchainRTView;
    ID3D11UnorderedAccessView* swapchainUAView;
    std::vector<ID3D11RenderTargetView*> renderTargetViews;
    std::vector<ID3D11UnorderedAccessView*> uaViews;
	
    ID3D11DepthStencilView* depthStencilBuffer;


    std::mutex ctxLock;

    std::unordered_map<uint64_t, ID3D11BlendState*>		    hashBS;
    std::unordered_map<uint64_t, ID3D11DepthStencilState*>  hashDSS;
    std::unordered_map<uint64_t, ID3D11RasterizerState*>	hashRS;
    std::unordered_map<uint64_t, ID3D11SamplerState*>	    hashSS;

    ID3D11BlendState* FetchBlendState(const Compressed::CoreBlendDesc& blendState);
    ID3D11DepthStencilState* FetchDepthStencilState(const Compressed::DepthStencilStateDesc& depthStencilState);
    ID3D11RasterizerState*FetchRasterizerState(const Compressed::RasterizerStateDesc& rasterizerState);
    ID3D11SamplerState* FetchSamplerState(const Compressed::SamplerStateDesc& state);

#pragma endregion 

    
protected:

    IResource* CreateResource(const GpuResource& ResourceDesc) override;
    void DestroyResource(IResource* const resource) override;
    void DestroyResourceView(IResourceView* const resource) override;
    IResourceView* CreateResourceView(const GpuResourceView& desc, const GpuResource& ResourceDesc) override;

    IShader* CreateShader(const ShaderDesc& desc) override;
    IInputLayout* CreateInputLayout(const InputAssemblerDeclarationDesc& desc, const ShaderDesc& Shader) override;
    
    void SetResourceData(const GpuResource& resource, uint16_t dstSubresource, const UBox& rect, const void* pSrcData, int32_t srcRowPitch, int32_t srcDepthPitch) override;

     virtual void Draw(const DrawCall& call) override;
    void Present() override;

#pragma region SetupPipeline
    
    void SetupViewports(const Compressed::ViewportDesc viewports[], uint8_t num);
    void SetupBlendState(const Compressed::CoreBlendDesc& blendState);
    void SetupDepthStencilState(const Compressed::DepthStencilStateDesc& depthStencilState);
    void SetupRasterizerState(const Compressed::RasterizerStateDesc& rasterizerState);
    void SetupSamplers(const Compressed::SamplerStateDesc samplers[], uint8_t num);
    void SetupPrimitiveTopology(const EPrimitiveTopology topology);
    void SetupShader(IShader* shader, EShaderType type);
    void SetupInputLayout(IInputLayout* layout);

    virtual void SetupPipeline(const PipelineDescription& Pipeline) override;
    virtual void SetupVertexBuffers(IVertexBufferView* const vertexBuffers[], uint8_t num) override;
    virtual void SetupIndexBuffers(IIndexBufferView* const indices) override;
    virtual void SetupTextures(IResourceView* textures[], uint8_t num)  override;
    virtual void SetupRenderTargets(IRenderTargetView* const renderTargets[], int32_t num, IDepthStencilView* depthStencilBuffer)  override;
    virtual void SetupUATargets(IUATargetView* uaTargets[], uint8_t num) override;
    virtual void SetupConstBuffers(IConstBufferView* constBuffers[], uint8_t num) override;

    void ClearState() override;
    
    void ClearRenderTarget(IRenderTargetView* rtView, FColor color) override;
    void ClearDepthStencil(IDepthStencilView* dsView, float depth, int8_t stencil) override;


#pragma endregion

    void ResizeBackbuffer(int32_t width, int32_t height) override;

    void BeginEvent(const char* name) override;
    void EndEvent() override;

    void* GetNativeTexture(IResourceView* view) override;
public:
    
    void GetBackbufferSize(uint32_t& w, uint32_t& h) override;
protected:
    void SyncBlockExecutionStart() override;
    void SyncResourcesRead(IResource** data, size_t size) override;
    void SyncResourcesWrite(IResource** data, size_t size) override;
    void SyncBlockExecutionEnd() override;
};

}
