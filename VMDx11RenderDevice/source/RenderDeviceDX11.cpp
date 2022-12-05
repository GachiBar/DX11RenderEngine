
#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include "RenderDeviceDX11.h"

#include "d3d.h"

#include "BuffersViews/ConstBufferViewD3D11.h"
#include "BuffersViews/IndexBufferViewD3D11.h"
#include "BuffersViews/VertexBufferViewD3D11.h"
#include "GraphicsExceptions/GraphicsException.h"
#include "GPUResourcesDescription/GPUResource.h"


#include "GraphicsExceptions/GraphicsExceptionsDefines.h"

using namespace GVM;

RenderDeviceDX11::RenderDeviceDX11(const RenderDeviceInitParams& initParams, bool debugMode)
    : IRenderDevice(initParams, debugMode)
{
    //DXGI_ADAPTER_DESC adapterDesc;
    D3D_FEATURE_LEVEL levels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    uint32_t flags;

    /* Load CreateDXGIFactory1 */
    //GFX_THROW_INFO(CreateDXGIFactory(__uuidof(IDXGIFactory), &factory));
    //factory->EnumAdapters(0, adapter.GetAddressOf());
    //
    //adapter->GetDesc(&adapterDesc);

    /* Create the D3D11Device */
    flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if _DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif


    DXGI_SWAP_CHAIN_DESC swapchainDesc;

    /* Initialize swapchain descriptor */
    swapchainDesc.BufferDesc.Width = 0;
    swapchainDesc.BufferDesc.Height = 0;
    swapchainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.BufferCount = 3;
    swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    //swapchainDesc.BufferDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = 0;
    swapchainDesc.OutputWindow = ((Dx11PlatformHandle*)(initParams.deviceWindowHandle))->hwnd;

    /* Create the swap chain! */
    //GFX_THROW_INFO(factory->CreateSwapChain(
    //	device.Get(),
    //	&swapchainDesc,
    //	&swapchain
    //));

    GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
        NULL, /* FIXME: Should be renderer->adapter! */
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags,
        NULL,
        0,
        D3D11_SDK_VERSION,
        &swapchainDesc,
        &swapchain,
        &device,
        &featureLevel,
        &context
    ));


    /* Initialize texture and sampler collections */
    Textures.resize(MAX_TEXTURE_ATTACHMENTS, nullptr);

    renderTargetViews.resize(MAX_RENDERTARGET_ATTACHMENTS);
    uaViews.resize(MAX_RENDERTARGET_ATTACHMENTS);

    /* Initialize SetStringMarker support, if available */
    //if (featureLevel == D3D_FEATURE_LEVEL_11_1) {
    //	context->QueryInterface(&annotation);
    //
    //	res = ID3D11DeviceContext_QueryInterface(
    //		renderer->context,
    //		&D3D_IID_ID3DUserDefinedAnnotation,
    //		(void**)&renderer->annotation
    //	);
    //	ERROR_CHECK("Could not get UserDefinedAnnotation")
    //} else {
    //	FNA3D_LogInfo("SetStringMarker not supported!");
    //}

    /* Initialize renderer members not covered by SDL_memset('\0') */
    this->debugMode = debugMode;
    blendFactor = FColor(1, 1, 1, 1);
    multiSampleMask = -1; /* AKA 0xFFFFFFFF, ugh -flibit */
    topology = EPrimitiveTopology::PRIMITIVE_TOPOLOGY_UNKNOWN; /* Force an update */


    context->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), &perf);

    /* Set presentation interval */
    switch (initParams.presentationInterval)
    {
    case EPresentInterval::PRESENT_INTERVAL_ONE:
    case EPresentInterval::PRESENT_INTERVAL_DEFAULT:
    {
        syncInterval = 1;
        break;
    }
    case EPresentInterval::PRESENT_INTERVAL_TWO:
    {
        syncInterval = 2;
        break;
    }
    case EPresentInterval::PRESENT_INTERVAL_IMMEDIATE:
    {
        syncInterval = 0;
        break;
    }
    }


    D3D11_RENDER_TARGET_VIEW_DESC swapchainViewDesc;
    wrl::ComPtr<ID3D11Texture2D> swapchainTexture;


    backBufferWidth = initParams.backBufferWidth;
    backBufferHeight = initParams.backBufferHeight;

    /* Create a render target view for the swapchain */
    swapchainViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    swapchainViewDesc.Texture2D.MipSlice = 0;

    GFX_THROW_INFO(swapchain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        &swapchainTexture
    ));

    GFX_THROW_INFO(device->CreateRenderTargetView(
        swapchainTexture.Get(),
        NULL,
        &swapchainRTView
    ));

    
    this->vertexBuffers.resize(32);
    this->vertexBufferOffsets.resize(32);
    this->vertexBufferStrides.resize(32);
}

RenderDeviceDX11::~RenderDeviceDX11()
{}

constexpr DXGI_FORMAT ToD3D_TextureFormat[] =
{
    DXGI_FORMAT_UNKNOWN,

    DXGI_FORMAT_R32G32B32A32_TYPELESS,
    DXGI_FORMAT_R32G32B32A32_FLOAT,
    DXGI_FORMAT_R32G32B32A32_UINT,
    DXGI_FORMAT_R32G32B32A32_SINT,
    DXGI_FORMAT_R16G16B16A16_TYPELESS,
    DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_UNORM,
    DXGI_FORMAT_R16G16B16A16_UINT,
    DXGI_FORMAT_R16G16B16A16_SNORM,
    DXGI_FORMAT_R16G16B16A16_SINT,
    DXGI_FORMAT_R10G10B10A2_TYPELESS,
    DXGI_FORMAT_R10G10B10A2_UNORM,
    DXGI_FORMAT_R10G10B10A2_UINT,
    DXGI_FORMAT_R8G8B8A8_TYPELESS,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_R8G8B8A8_UINT,
    DXGI_FORMAT_R8G8B8A8_SNORM,
    DXGI_FORMAT_R8G8B8A8_SINT,
    //3 chanel
    DXGI_FORMAT_R32G32B32_TYPELESS,
    DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32_UINT,
    DXGI_FORMAT_R32G32B32_SINT,
    //2 chanel
    DXGI_FORMAT_R32G32_TYPELESS,
    DXGI_FORMAT_R32G32_FLOAT,
    DXGI_FORMAT_R32G32_UINT,
    DXGI_FORMAT_R32G32_SINT,
    DXGI_FORMAT_R16G16_TYPELESS,
    DXGI_FORMAT_R16G16_FLOAT,
    DXGI_FORMAT_R16G16_UNORM,
    DXGI_FORMAT_R16G16_UINT,
    DXGI_FORMAT_R16G16_SNORM,
    DXGI_FORMAT_R16G16_SINT,
    DXGI_FORMAT_R8G8_TYPELESS,
    DXGI_FORMAT_R8G8_UNORM,
    DXGI_FORMAT_R8G8_UINT,
    DXGI_FORMAT_R8G8_SNORM,
    DXGI_FORMAT_R8G8_SINT,
    //1 chanel
    DXGI_FORMAT_R32_TYPELESS,
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R32_UINT,
    DXGI_FORMAT_R32_SINT,
    DXGI_FORMAT_R16_TYPELESS,
    DXGI_FORMAT_R16_FLOAT,
    DXGI_FORMAT_R16_UNORM,
    DXGI_FORMAT_R16_UINT,
    DXGI_FORMAT_R16_SNORM,
    DXGI_FORMAT_R16_SINT,
    DXGI_FORMAT_R8_TYPELESS,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8_UINT,
    DXGI_FORMAT_R8_SNORM,
    DXGI_FORMAT_R8_SINT,
    DXGI_FORMAT_A8_UNORM,
};

constexpr DXGI_FORMAT ToD3D_DepthFormat[] = {
    DXGI_FORMAT_UNKNOWN,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    DXGI_FORMAT_D16_UNORM,
    DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
};

UINT ToD3D11ResourceBinDings(uint32_t bindings)
{
    UINT result = 0;
    constexpr std::pair<uint32_t, D3D11_BIND_FLAG> BindFlags[] = {
        {to_underlying(EBindFlags::BIND_VERTEX_BUFFER),D3D11_BIND_VERTEX_BUFFER},
        {to_underlying(EBindFlags::BIND_INDEX_BUFFER),D3D11_BIND_INDEX_BUFFER},
        {to_underlying(EBindFlags::BIND_CONSTANT_BUFFER),D3D11_BIND_CONSTANT_BUFFER},
        {to_underlying(EBindFlags::BIND_SHADER_RESOURCE),D3D11_BIND_SHADER_RESOURCE},
        {to_underlying(EBindFlags::BIND_STREAM_OUTPUT),D3D11_BIND_STREAM_OUTPUT},
        {to_underlying(EBindFlags::BIND_RENDER_TARGET),D3D11_BIND_RENDER_TARGET},
        {to_underlying(EBindFlags::BIND_DEPTH_STENCIL),D3D11_BIND_DEPTH_STENCIL},
        {to_underlying(EBindFlags::BIND_UNORDERED_ACCESS),D3D11_BIND_UNORDERED_ACCESS},
    };
    for (auto& flag : BindFlags)
    {
        if (bindings & flag.first)
            result |= flag.second;
    }
    return result;
}

IRenderDevice::IResource* RenderDeviceDX11::CreateResource(const GpuResource& ResourceDesc)
{
    switch (ResourceDesc.resourceDescription.Dimension)
    {
    case EResourceDimension::RESOURCE_DIMENSION_TEXTURE2D:
    {
        ID3D11Texture2D* result;

        D3D11_TEXTURE2D_DESC dx11desc;

        auto& desc = ResourceDesc.resourceDescription;

        /* Initialize descriptor */
        dx11desc.Width = desc.Width;
        dx11desc.Height = desc.Height;
        dx11desc.MipLevels = 1;
        dx11desc.ArraySize = desc.DepthOrArraySize;
        dx11desc.Format = ToD3D_TextureFormat[to_underlying(desc.Format)];
        dx11desc.SampleDesc.Count = 1;
        dx11desc.SampleDesc.Quality = 0;
        dx11desc.Usage = D3D11_USAGE_DEFAULT;
        dx11desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | ToD3D11ResourceBinDings(ResourceDesc.resourceBindings);
        dx11desc.CPUAccessFlags = 0;
        dx11desc.MiscFlags = 0;

        //   ResourceDesc.resourceBindings;
        //if (isRenderTarget)
        // {
        //    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        //   desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        // }

        /* Create the texture */
        GFX_THROW_INFO(device->CreateTexture2D(
            &dx11desc,
            NULL,
            &result
        ));

        return reinterpret_cast<IRenderDevice::IResource*>(result);
    }
    case EResourceDimension::RESOURCE_DIMENSION_BUFFER:
    {
        ID3D11Buffer* result;

        D3D11_BUFFER_DESC dx11desc;

        auto& desc = ResourceDesc.resourceDescription;

        /* Initialize descriptor */
        dx11desc.ByteWidth = desc.Width;
        dx11desc.Usage = D3D11_USAGE_DEFAULT;
        dx11desc.BindFlags = ToD3D11ResourceBinDings(ResourceDesc.resourceBindings);
        if (!(dx11desc.BindFlags & D3D11_BIND_CONSTANT_BUFFER))
        {
            dx11desc.BindFlags |= D3D11_BIND_INDEX_BUFFER ;
            dx11desc.BindFlags |= D3D11_BIND_VERTEX_BUFFER ;
        }
        dx11desc.CPUAccessFlags = 0;
        dx11desc.MiscFlags = 0;

        //   ResourceDesc.resourceBindings;
        //if (isRenderTarget)
        // {
        //    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        //   desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        // }

        /* Create the texture */
        GFX_THROW_INFO(device->CreateBuffer(
            &dx11desc,
            NULL,
            &result
        ));

        return reinterpret_cast<IRenderDevice::IResource*>(result);
    }
    }
    return nullptr;
}

void RenderDeviceDX11::DestroyResource(IResource* const resource)
{
    IUnknown* d3d11resource = reinterpret_cast<IUnknown*>(resource);
    if (d3d11resource)
        d3d11resource->Release();
}


void RenderDeviceDX11::DestroyResourceView(IResourceView* const resource)
{
    IUnknown* d3d11resource = reinterpret_cast<IUnknown*>(resource);
    if (d3d11resource)
        d3d11resource->Release();
}


D3D11_DEPTH_STENCIL_VIEW_DESC ToD3D11DepthStencilView(const DepthStencilViewDesc& desc)
{
    D3D11_DEPTH_STENCIL_VIEW_DESC result;
    result.Flags = to_underlying(desc.Flag);
    result.Format = ToD3D_DepthFormat[to_underlying(desc.Format)];
    result.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    result.Texture2D.MipSlice = 0;
    return result;
}


constexpr D3D_SRV_DIMENSION ToD3D_ShaderViewDimension[] = {

    D3D_SRV_DIMENSION_UNKNOWN,
    D3D_SRV_DIMENSION_TEXTURE1D,
    D3D_SRV_DIMENSION_TEXTURE1DARRAY,
    D3D_SRV_DIMENSION_TEXTURE2D,
    D3D_SRV_DIMENSION_TEXTURE2DARRAY,
    D3D_SRV_DIMENSION_TEXTURE3D,
    D3D_SRV_DIMENSION_TEXTURECUBE,
    D3D_SRV_DIMENSION_TEXTURECUBEARRAY,
};

constexpr D3D11_RTV_DIMENSION ToD3D_RTViewDimension[] = {
    D3D11_RTV_DIMENSION_UNKNOWN,
    D3D11_RTV_DIMENSION_TEXTURE1D,
    D3D11_RTV_DIMENSION_TEXTURE1DARRAY,
    D3D11_RTV_DIMENSION_TEXTURE2D,
    D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
    D3D11_RTV_DIMENSION_TEXTURE3D,
};


constexpr D3D11_UAV_DIMENSION ToD3D_UAViewDimension[] = {
    D3D11_UAV_DIMENSION_UNKNOWN,
    D3D11_UAV_DIMENSION_TEXTURE1D,
    D3D11_UAV_DIMENSION_TEXTURE1DARRAY,
    D3D11_UAV_DIMENSION_TEXTURE2D,
    D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
    D3D11_UAV_DIMENSION_TEXTURE3D,
};

D3D11_SHADER_RESOURCE_VIEW_DESC ToD3D11ShaderView(const ShaderResourceViewDesc& desc)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC result;
    result.Format = ToD3D_TextureFormat[to_underlying(desc.Format)];
    result.ViewDimension = ToD3D_ShaderViewDimension[to_underlying(desc.Dimension)];
    result.Texture2D.MipLevels = 1;
    result.Texture2D.MostDetailedMip = 0;
    return result;
}

D3D11_RENDER_TARGET_VIEW_DESC ToD3D11RTView(const RenderTargetViewDesc& desc)
{
    D3D11_RENDER_TARGET_VIEW_DESC result;
    result.Format = ToD3D_TextureFormat[to_underlying(desc.Format)];
    result.ViewDimension = ToD3D_RTViewDimension[to_underlying(desc.Dimension)];
    result.Texture2D.MipSlice = 0;
    return result;
}

D3D11_UNORDERED_ACCESS_VIEW_DESC ToD3D11UAView(const UATargetViewDesc& desc)
{
    D3D11_UNORDERED_ACCESS_VIEW_DESC result;
    result.Format = ToD3D_TextureFormat[to_underlying(desc.Format)];
    result.ViewDimension = ToD3D_UAViewDimension[to_underlying(desc.Dimension)];
    result.Texture2D.MipSlice = 0;
    return result;
}

IRenderDevice::IResourceView* RenderDeviceDX11::CreateResourceView(const GpuResourceView& desc, const GpuResource& ResourceDesc)
{
    switch (desc.type)
    {
    case GpuResourceView::EViewType::CB:
    {
        ConstBufferViewD3D11* result = new ConstBufferViewD3D11(desc.cbViewDescription, reinterpret_cast<ID3D11Buffer*>(ResourceDesc.resource));
        return reinterpret_cast<IResourceView*>(result);
        break;
    }
    case GpuResourceView::EViewType::VB:
    {
        VertexBufferViewD3D11* result = new VertexBufferViewD3D11(desc.vbViewDescription, reinterpret_cast<ID3D11Buffer*>(ResourceDesc.resource));
        return reinterpret_cast<IResourceView*>(result);
        break;
    }
    case GpuResourceView::EViewType::IB:
    {
        IndexBufferViewD3D11* result = new IndexBufferViewD3D11(desc.ibViewDescription, reinterpret_cast<ID3D11Buffer*>(ResourceDesc.resource));
        return reinterpret_cast<IResourceView*>(result);
        break;
    }
    case GpuResourceView::EViewType::DB:
    {
        ID3D11DepthStencilView* result;
        auto d3d11desc = ToD3D11DepthStencilView(desc.dbViewDescription);
        device->CreateDepthStencilView(
            reinterpret_cast<ID3D11Resource*>(ResourceDesc.resource),
            &d3d11desc,
            &result
        );
        return reinterpret_cast<IResourceView*>(result);
        break;
    }
    case GpuResourceView::EViewType::RT:
    {
        ID3D11RenderTargetView* result;
        if (desc.rtViewDescription.MakeDefault)
        {
            device->CreateRenderTargetView(
                reinterpret_cast<ID3D11Resource*>(ResourceDesc.resource),
                nullptr,
                &result
            );
        }
        else
        {
            auto d3d11desc = ToD3D11RTView(desc.rtViewDescription);
            device->CreateRenderTargetView(
                reinterpret_cast<ID3D11Resource*>(ResourceDesc.resource),
                &d3d11desc,
                &result
            );
        }

        return reinterpret_cast<IResourceView*>(result);
        break;
    }
    case GpuResourceView::EViewType::SR:
    {
        ID3D11ShaderResourceView* result;
        if (desc.srViewDescription.MakeDefault)
        {
            device->CreateShaderResourceView(
                reinterpret_cast<ID3D11Resource*>(ResourceDesc.resource),
                nullptr,
                &result
            );
        }
        else
        {
            auto d3d11desc = ToD3D11ShaderView(desc.srViewDescription);
            device->CreateShaderResourceView(
                reinterpret_cast<ID3D11Resource*>(ResourceDesc.resource),
                &d3d11desc,
                &result
            );
        }

        return reinterpret_cast<IResourceView*>(result);
        break;
    }
    case GpuResourceView::EViewType::UA:
    {
        ID3D11UnorderedAccessView* result;
        if (desc.rtViewDescription.MakeDefault)
        {
            device->CreateUnorderedAccessView(
                reinterpret_cast<ID3D11Resource*>(ResourceDesc.resource),
                nullptr,
                &result
            );
        }
        else
        {
            auto d3d11desc = ToD3D11UAView(desc.uaViewDescription);
            device->CreateUnorderedAccessView(
                reinterpret_cast<ID3D11Resource*>(ResourceDesc.resource),
                &d3d11desc,
                &result
            );
        }

        return reinterpret_cast<IResourceView*>(result);
        break;
    }
    }
    return nullptr;
}


IRenderDevice::IShader* RenderDeviceDX11::CreateShader(const ShaderDesc& desc)
{
    ID3D11DeviceChild* result = nullptr;
    switch (desc.type)
    {
    case EShaderType::HULL_SHADER:
    {
        ID3D11HullShader* hs;
        GFX_THROW_INFO(device->CreateHullShader(desc.bytecode, desc.byteCodeSize, nullptr, &hs));
        result = hs;
        break;
    }
    case EShaderType::PIXEL_SHADER:
    {
        ID3D11PixelShader* ps;
        GFX_THROW_INFO(device->CreatePixelShader(desc.bytecode, desc.byteCodeSize, nullptr, &ps));
        result = ps;
        break;
    }
    case EShaderType::DOMAIN_SHADER:
    {
        ID3D11DomainShader* ds;
        GFX_THROW_INFO(device->CreateDomainShader(desc.bytecode, desc.byteCodeSize, nullptr, &ds));
        result = ds;
        break;
    }
    case EShaderType::VERTEX_SHADER:
    {
        ID3D11VertexShader* vs;
        GFX_THROW_INFO(device->CreateVertexShader(desc.bytecode, desc.byteCodeSize, nullptr, &vs));
        result = vs;
        break;
    }
    case EShaderType::COMPUTE_SHADER:
    {
        ID3D11ComputeShader* cs;
        GFX_THROW_INFO(device->CreateComputeShader(desc.bytecode, desc.byteCodeSize, nullptr, &cs));
        result = cs;
        break;
    }
    case EShaderType::GEOMETRY_SHADER:
    {
        ID3D11GeometryShader* gs;
        GFX_THROW_INFO(device->CreateGeometryShader(desc.bytecode, desc.byteCodeSize, nullptr, &gs));
        result = gs;
        break;
    }
    default:
    {
        break;
    };
    }
    // GFX_THROW_INFO(
    //           );
    return reinterpret_cast<IRenderDevice::IShader*>(result);
}

std::vector<D3D11_INPUT_ELEMENT_DESC> ToD3D11(const InputAssemblerDeclarationDesc& desc)
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> result(desc.InputElementDescs.size());
    for (int i = 0; i < desc.InputElementDescs.size(); i++)
    {
        result[i].Format = ToD3D_TextureFormat[to_underlying(desc.InputElementDescs[i].Format)];
        result[i].InputSlot = desc.InputElementDescs[i].InputSlot;
        result[i].SemanticIndex = desc.InputElementDescs[i].SemanticIndex;
        result[i].SemanticName = desc.InputElementDescs[i].SemanticName;
        result[i].AlignedByteOffset = desc.InputElementDescs[i].AlignedByteOffset;
        result[i].InputSlotClass = D3D11_INPUT_CLASSIFICATION(to_underlying(desc.InputElementDescs[i].InputSlotClass));
        result[i].InstanceDataStepRate = desc.InputElementDescs[i].InstanceDataStepRate;
    }
    return result;
}

IRenderDevice::IInputLayout* RenderDeviceDX11::CreateInputLayout(const InputAssemblerDeclarationDesc& desc, const ShaderDesc& Shader)
{
    auto d3d11InputDesc = ToD3D11(desc);
    ID3D11InputLayout* inputLayout;

    device->CreateInputLayout(
        d3d11InputDesc.data(),
        d3d11InputDesc.size(),
        Shader.bytecode,
        Shader.byteCodeSize,
        &inputLayout);

    return reinterpret_cast<IRenderDevice::IInputLayout*>(inputLayout);
}
void RenderDeviceDX11::SetResourceData(const GpuResource& resource, uint16_t dstSubresource, const UBox& rect, const void* pSrcData, int32_t srcRowPitch,
    int32_t srcDepthPitch)
{
    D3D11_BOX dstBox;
    
       

    //int32_t blockSize = Texture_GetBlockSize(d3dTexture->format);

    ///if (blockSize > 1) {
    //	w = (w + blockSize - 1) & ~(blockSize - 1);
    //	h = (h + blockSize - 1) & ~(blockSize - 1);
    //}
    if (resource.resourceBindings & to_underlying(EBindFlags::BIND_CONSTANT_BUFFER))
    {
        GFX_THROW_INFO_ONLY(context->UpdateSubresource(
            reinterpret_cast<ID3D11Resource*>(resource.resource),
            dstSubresource,
            nullptr,
            pSrcData,
            srcRowPitch,
            srcDepthPitch
        ));
    }else
    {
        dstBox.left = rect.Left;
        dstBox.top = rect.Top;
        dstBox.front = rect.Front;
        dstBox.right = rect.Right;
        dstBox.bottom = rect.Bottom;
        dstBox.back = rect.Back;

        GFX_THROW_INFO_ONLY(context->UpdateSubresource(
                    reinterpret_cast<ID3D11Resource*>(resource.resource),
                    dstSubresource,
                    &dstBox,
                    pSrcData,
                    srcRowPitch,
                    srcDepthPitch
                ));
    }
    //SDL_LockMutex(renderer->ctxLock);
}

bool ToD3D11Viewports(const Compressed::ViewportDesc viewports[], D3D11_VIEWPORT d3d11viewports[20], uint8_t num)
{
    bool result = false;
    for (int i = 0; i < num; i++)
    {
        if (((D3D11_VIEWPORT*)viewports)[i] != d3d11viewports[i])
        {
            d3d11viewports[i] = ((D3D11_VIEWPORT*)viewports)[i];
            result = true;
        }
        assert(-32768.000000 <= d3d11viewports[i].TopLeftX &&
        -32768.000000 <= d3d11viewports[i].TopLeftY &&
        -32768.000000 <= (d3d11viewports[i].TopLeftX+d3d11viewports[i].Width) &&
        (d3d11viewports[i].TopLeftX+d3d11viewports[i].Width) <= 32767.000000 &&
        (d3d11viewports[i].TopLeftY+d3d11viewports[i].Height) <= 32767.000000 &&
        0.000000 <= d3d11viewports[i].MinDepth &&
        d3d11viewports[i].MaxDepth <= 1.000000 &&
         d3d11viewports[i].MinDepth <= d3d11viewports[i].MaxDepth
        );
    }
    return result;
}

void RenderDeviceDX11::SetupViewports(const Compressed::ViewportDesc viewports[], uint8_t num)
{
    if (ToD3D11Viewports(viewports, d3d11viewports, num) || num != d3d11viewportsNum)
    {
        d3d11viewportsNum = num;
        GFX_THROW_INFO_ONLY(context->RSSetViewports(num, d3d11viewports));
    }
}

ID3D11BlendState* RenderDeviceDX11::FetchBlendState(const Compressed::CoreBlendDesc& blendState)
{
    auto hash =
       (std::hash<uint64_t>{}(blendState.BlendStates[0].data)      )^(std::hash<uint64_t>{}(blendState.BlendStates[4].data) << 4u)^
       (std::hash<uint64_t>{}(blendState.BlendStates[1].data) >> 1u)^(std::hash<uint64_t>{}(blendState.BlendStates[5].data) << 1u)^
       (std::hash<uint64_t>{}(blendState.BlendStates[2].data) >> 2u)^(std::hash<uint64_t>{}(blendState.BlendStates[6].data) << 2u)^
       (std::hash<uint64_t>{}(blendState.BlendStates[3].data) >> 3u)^(std::hash<uint64_t>{}(blendState.BlendStates[7].data) << 3u);
    if (!hashBS.contains(hash))
    {
        ID3D11BlendState* bs;
        CD3D11_BLEND_DESC desc{CD3D11_DEFAULT()};
        desc.AlphaToCoverageEnable = 0;
        desc.IndependentBlendEnable = 0;
        
        for (int i =0; i< 8; i++)
        {
            auto& tbs = blendState.BlendStates[i];
            BlendStateDesc test = GVM::BlendStateDesc(tbs);
            desc.RenderTarget[i].BlendEnable = tbs.Fields.BlendEnable &&
                !(
                    tbs.Fields.SrcBlend == D3D11_BLEND_ONE &&
                    tbs.Fields.DestBlend == D3D11_BLEND_ZERO &&
                    tbs.Fields.SrcBlendAlpha == D3D11_BLEND_ONE &&
                    tbs.Fields.DestBlendAlpha == D3D11_BLEND_ZERO
                );
            if (desc.RenderTarget[i].BlendEnable)
            {
                desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP((tbs.Fields.BlendOp));
                desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP((tbs.Fields.BlendOpAlpha));
                desc.RenderTarget[i].DestBlend = D3D11_BLEND((tbs.Fields.DestBlend));
                desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND((tbs.Fields.DestBlendAlpha));
                desc.RenderTarget[i].SrcBlend = D3D11_BLEND((tbs.Fields.SrcBlend));
                desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND((tbs.Fields.SrcBlendAlpha));
            }
        }

        device->CreateBlendState(&desc, &bs);
        hashBS.insert({hash,bs});
    }
    return hashBS[hash];
}

void RenderDeviceDX11::SetupBlendState(const Compressed::CoreBlendDesc& blendState)
{
    static FLOAT BlendFactor[4];
    blendFactor = {
        blendState.desc.BlendFactor[0],
        blendState.desc.BlendFactor[1],
        blendState.desc.BlendFactor[2],
        blendState.desc.BlendFactor[3]
    };

    auto bs = FetchBlendState(blendState);
    if (bs!=blendStatePtr || blendFactor != blendFactorHash || blendState.desc.SampleMask != multiSampleMask)
    {
        blendStatePtr = bs;
        multiSampleMask = blendState.desc.SampleMask;
        blendFactorHash = blendFactor;
        context->OMSetBlendState(bs, blendFactor.Color, blendState.desc.SampleMask);
    }
}

D3D11_DEPTH_STENCIL_DESC ToDX11DSState(const Compressed::DepthStencilStateDesc& depthStencilState)
{
    const std::array<D3D11_STENCIL_OP, 9> depthOpArray = {
        D3D11_STENCIL_OP(0),
        D3D11_STENCIL_OP_KEEP,
        D3D11_STENCIL_OP_ZERO,
        D3D11_STENCIL_OP_REPLACE,
        D3D11_STENCIL_OP_INCR_SAT,
        D3D11_STENCIL_OP_DECR_SAT,
        D3D11_STENCIL_OP_INVERT,
        D3D11_STENCIL_OP_INCR,
        D3D11_STENCIL_OP_DECR
    };

    const std::array<D3D11_COMPARISON_FUNC, 9> depthCompArray = {
        D3D11_COMPARISON_FUNC(0),
        D3D11_COMPARISON_NEVER,
        D3D11_COMPARISON_LESS,
        D3D11_COMPARISON_EQUAL,
        D3D11_COMPARISON_LESS_EQUAL,
        D3D11_COMPARISON_GREATER,
        D3D11_COMPARISON_NOT_EQUAL,
        D3D11_COMPARISON_GREATER_EQUAL,
        D3D11_COMPARISON_ALWAYS
    };


    CD3D11_DEPTH_STENCIL_DESC desc{CD3D11_DEFAULT()};
    desc.BackFace.StencilFailOp = depthOpArray[depthStencilState.Fields.BackStencilFailOp];
    desc.BackFace.StencilFunc = depthCompArray[depthStencilState.Fields.BackStencilFunc];
    desc.BackFace.StencilPassOp = depthOpArray[depthStencilState.Fields.BackStencilPassOp];
    desc.BackFace.StencilDepthFailOp = depthOpArray[depthStencilState.Fields.BackStencilDepthFailOp];

    desc.BackFace.StencilFailOp = depthOpArray[depthStencilState.Fields.FrontStencilFailOp];
    desc.BackFace.StencilFunc = depthCompArray[depthStencilState.Fields.FrontStencilFunc];
    desc.BackFace.StencilPassOp = depthOpArray[depthStencilState.Fields.FrontStencilPassOp];
    desc.BackFace.StencilDepthFailOp = depthOpArray[depthStencilState.Fields.FrontStencilDepthFailOp];

    desc.DepthEnable = depthStencilState.Fields.DepthEnable;
    desc.DepthFunc = depthCompArray[depthStencilState.Fields.DepthFunc];
    desc.StencilEnable = depthCompArray[depthStencilState.Fields.StencilEnable];
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK(depthStencilState.Fields.DepthWriteMask);
    desc.StencilWriteMask = depthStencilState.Fields.StencilWriteMask;
    desc.StencilReadMask = depthStencilState.Fields.StencilReadMask;

    return desc;
}

ID3D11DepthStencilState* RenderDeviceDX11::FetchDepthStencilState(const Compressed::DepthStencilStateDesc& depthStencilState)
{
    if (hashDSS.contains(depthStencilState.data))
    {
        ID3D11DepthStencilState* ds;
        auto desc = ToDX11DSState(depthStencilState);

        device->CreateDepthStencilState(&desc, &ds);
        hashDSS.insert({depthStencilState.data,ds});
    }
    return hashDSS[depthStencilState.data];
}

void RenderDeviceDX11::SetupDepthStencilState(const Compressed::DepthStencilStateDesc& depthStencilState)
{
    auto ds = FetchDepthStencilState(depthStencilState);
    if (ds != depthStencilStatePtr)
    {
        depthStencilStatePtr = ds;
        context->OMSetDepthStencilState(depthStencilStatePtr, 0);
    }
}



D3D11_RASTERIZER_DESC ToDX11RSState(const Compressed::RasterizerStateDesc& rasterizerState)
{
    CD3D11_RASTERIZER_DESC result{CD3D11_DEFAULT()};
    result.CullMode =
        std::array<D3D11_CULL_MODE, 4>({
            D3D11_CULL_NONE,
            D3D11_CULL_NONE,
            D3D11_CULL_FRONT,
            D3D11_CULL_BACK
        })
        [rasterizerState.Fields.CullMode];
    result.FillMode =
        std::array<D3D11_FILL_MODE, 3>({
            D3D11_FILL_MODE(0),
            D3D11_FILL_WIREFRAME,
            D3D11_FILL_SOLID
        })
        [rasterizerState.Fields.FillMode];
    result.FrontCounterClockwise = rasterizerState.Fields.FrontCounterClockwise;
    result.DepthClipEnable = rasterizerState.Fields.DepthClipEnable;
    result.ScissorEnable = rasterizerState.Fields.ScissorEnable;
    result.DepthBiasClamp = D3D11_FLOAT32_MAX;

    return result;
}

ID3D11RasterizerState* RenderDeviceDX11::FetchRasterizerState(const Compressed::RasterizerStateDesc& rasterizerState)
{
    if (!hashRS.contains(rasterizerState.data))
    {
        ID3D11RasterizerState* rs;
        D3D11_RASTERIZER_DESC desc = ToDX11RSState(rasterizerState);

        device->CreateRasterizerState(&desc, &rs);
        hashRS.insert({rasterizerState.data,rs});
    }
    return hashRS[rasterizerState.data];
}


void RenderDeviceDX11::SetupRasterizerState(const Compressed::RasterizerStateDesc& rasterizerState)
{
    auto rs = FetchRasterizerState(rasterizerState);
    if (rs != rasterizerStatePtr)
    {
        rasterizerStatePtr = rs;
        context->RSSetState(rasterizerStatePtr);
    }
}

D3D11_FILTER ToD3D11SamplerFilter[] = {
    D3D11_FILTER_MIN_MAG_MIP_POINT,
    D3D11_FILTER_MIN_MAG_MIP_POINT,
    D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR,
    D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
    D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_MIN_MAG_MIP_LINEAR,
    D3D11_FILTER_ANISOTROPIC,
    D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
    D3D11_FILTER_COMPARISON_ANISOTROPIC,
    D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT,
    D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
    D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
    D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR,
    D3D11_FILTER_MINIMUM_ANISOTROPIC,
    D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT,
    D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
    D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
    D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
    D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
    D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR,
    D3D11_FILTER_MAXIMUM_ANISOTROPIC
};

ID3D11SamplerState* RenderDeviceDX11::FetchSamplerState(const Compressed::SamplerStateDesc& state)
{
    if (hashSS.contains(state.Data))
        return hashSS[state.Data];

    ID3D11SamplerState* istate;

    CD3D11_SAMPLER_DESC desc{CD3D11_DEFAULT()};
    assert(state.Fields.Filter != 0);
    desc.Filter = ToD3D11SamplerFilter[state.Fields.Filter];
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MaxAnisotropy = state.Fields.MaxAnisotropy;
    desc.MaxLOD = state.MaxLOD;
    desc.MinLOD = state.MinLOD;
    desc.MipLODBias = 0;

    device->CreateSamplerState(&desc, &istate);
    hashSS.insert({state.Data,istate});
    return hashSS[state.Data];
}


void RenderDeviceDX11::SetupSamplers(const Compressed::SamplerStateDesc samplers[], uint8_t num)
{
    bool flag = samplersNum != num;
    for (int i = 0; i < num; i++)
    {
        auto sampler = FetchSamplerState((samplers[i]));
        flag |= Samplers[i] != sampler;
        Samplers[i] = sampler;
    }
    if(flag)
    {
        samplersNum = num;
        if (CS) context->CSSetSamplers(0, num, Samplers);
        if (PS) context->PSSetSamplers(0, num, Samplers);
        if (GS) context->GSSetSamplers(0, num, Samplers);
        if (DS) context->DSSetSamplers(0, num, Samplers);
        if (HS) context->HSSetSamplers(0, num, Samplers);
        if (VS) context->VSSetSamplers(0, num, Samplers);
    }
}

D3D11_PRIMITIVE_TOPOLOGY ToD3DPT[] = {
    D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED,
    D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
    D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
    D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
    D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
    D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,
    D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,
    D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,
    D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST,
    D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST,
    D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,
};

void RenderDeviceDX11::SetupPrimitiveTopology(const EPrimitiveTopology topology)
{
    if (topologyHash != topology)
    {
        topologyHash = topology;
        context->IASetPrimitiveTopology(ToD3DPT[to_underlying(topology)]);
    }
}

void RenderDeviceDX11::SetupVertexBuffers(IVertexBufferView* const vertexBuffers[], uint8_t num)
{
    bool flag = num != vertexBuffersNum; 
    vertexBuffersNum = num;
    for (int i = 0; i < num; i++)
    {
        const VertexBufferViewD3D11* vb = reinterpret_cast<const VertexBufferViewD3D11*>(vertexBuffers[i]);
        if (vb != nullptr)
        {
            flag |= this->vertexBuffers[i] !=  vb->vertexBuffer;
            this->vertexBuffers[i] = vb->vertexBuffer;
            flag |= vertexBufferOffsets[i] != vb->Offset;
            vertexBufferOffsets[i] = vb->Offset;
            flag |= vertexBufferStrides[i] != vb->Stride;
            vertexBufferStrides[i] = vb->Stride;
        }
        else
        {
            flag |= this->vertexBuffers[i] !=  nullptr;
            this->vertexBuffers[i] = nullptr;
            flag |= vertexBufferOffsets[i] !=  0;
            vertexBufferOffsets[i] = 0;
            flag |= vertexBufferStrides[i] != 0;
            vertexBufferStrides[i] = 0;
        }
    }
    if (flag)
        context->IASetVertexBuffers(0, num, this->vertexBuffers.data(), vertexBufferStrides.data(), vertexBufferOffsets.data());
}

void RenderDeviceDX11::SetupIndexBuffers(IIndexBufferView* const indices)
{
    IndexBufferViewD3D11* indexBuffer = (IndexBufferViewD3D11*)(indices);
    if (indexBuffer != indexBufferPtr)
    {
        indexBufferPtr = indexBuffer;
        if (indices == nullptr) {
            GFX_THROW_INFO_ONLY(context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0));
        }
        else {
            GFX_THROW_INFO_ONLY(context->IASetIndexBuffer(indexBuffer->indexBuffer, indexBuffer->format, 0));
        }
    }
}

void RenderDeviceDX11::SetupTextures(IResourceView* textures[], uint8_t num)
{
    bool flag = num != texturesNum;
    if (textures == nullptr)
    {
        for (int i = 0; i < num; i++)
        {
            ID3D11ShaderResourceView* sh = nullptr;
            flag |= Textures[i] != nullptr;
            Textures[i] = sh;
        }
    }else
    {
        for (int i = 0; i < num; i++)
        {
            ID3D11ShaderResourceView* sh = (ID3D11ShaderResourceView*)(textures[i]);
            flag |= Textures[i] != sh;
            Textures[i] = sh;
        }
    }
    
    if (flag)
    {
        if (CS) context->CSSetShaderResources(0, num, Textures.data());
        if (PS) context->PSSetShaderResources(0, num, Textures.data());
        if (GS) context->GSSetShaderResources(0, num, Textures.data());
        if (DS) context->DSSetShaderResources(0, num, Textures.data());
        if (HS) context->HSSetShaderResources(0, num, Textures.data());
        if (VS) context->VSSetShaderResources(0, num, Textures.data());
    }
}

void RenderDeviceDX11::SetupRenderTargets(IRenderTargetView* const renderTargets[], int32_t num, IDepthStencilView* depthStencilBuffer)
{
    bool flag = num  != renderTargetsNum;
    flag |= this->depthStencilBuffer != (ID3D11DepthStencilView*)depthStencilBuffer;
    this->depthStencilBuffer = (ID3D11DepthStencilView*)depthStencilBuffer;
    if (renderTargets == nullptr)
    {
        static ID3D11RenderTargetView* nulls[MAX_RENDERTARGET_ATTACHMENTS];
        context->OMSetRenderTargets(MAX_RENDERTARGET_ATTACHMENTS, nulls, this->depthStencilBuffer);
        numRenderTargets = 0;
        return;
    }
    for (int i = 0; i < num; i++)
    {
        auto rtView =  renderTargets[i] == nullptr ? swapchainRTView : ((ID3D11RenderTargetView*)renderTargets[i]);
        flag |= renderTargetViews[i] != rtView;
        renderTargetViews[i] = rtView;
    }
    for (int i = num; i < MAX_RENDERTARGET_ATTACHMENTS; i++)
    {
        flag |= renderTargetViews[i] != nullptr;
        renderTargetViews[i] = nullptr;
    }
    if (flag)
    {
        context->OMSetRenderTargets(MAX_RENDERTARGET_ATTACHMENTS, renderTargetViews.data(), this->depthStencilBuffer);
    }
}

void RenderDeviceDX11::SetupUATargets(IUATargetView* uaTargets[], uint8_t num)
{
    static UINT arr[8];
    if (uaTargets == nullptr)
    {
        static ID3D11UnorderedAccessView* nulls[MAX_RENDERTARGET_ATTACHMENTS];
        context->CSSetUnorderedAccessViews(0, MAX_RENDERTARGET_ATTACHMENTS, nulls, arr);
        return;
    }
    for (int i = 0; i < num; i++)
    {
        uaViews[i] = ((ID3D11UnorderedAccessView*)uaTargets[i]);
    }
    for (int i = num; i < MAX_RENDERTARGET_ATTACHMENTS; i++)
    {
        uaViews[i] = nullptr;
    }

    context->CSSetUnorderedAccessViews(0, MAX_RENDERTARGET_ATTACHMENTS, uaViews.data(), arr);
}



void RenderDeviceDX11::SetupShader(IShader* shader, EShaderType type)
{
    switch (type)
    {
    case EShaderType::COMPUTE_SHADER:
    {
        if ((ID3D11ComputeShader*)shader != CSPtr)
        {
            CSPtr = (ID3D11ComputeShader*)shader;
            context->CSSetShader((ID3D11ComputeShader*)shader, nullptr, 0);
            CS = shader != nullptr;
        }
        break;
    }
    case EShaderType::HULL_SHADER:
        {
        if ((ID3D11ComputeShader*)shader != HSPtr)
        {
            HSPtr = (ID3D11HullShader*)shader;
            context->HSSetShader((ID3D11HullShader*)shader, nullptr, 0);
            HS = shader != nullptr;
        }
        break;
    }
    case EShaderType::PIXEL_SHADER:
    {
        if ((ID3D11ComputeShader*)shader != PSPtr)
        {
            PSPtr = (ID3D11ComputeShader*)shader;
            context->PSSetShader((ID3D11PixelShader*)shader, nullptr, 0);
            PS = shader != nullptr;
        }
        break;
    }
    case EShaderType::DOMAIN_SHADER:
    {
        if ((ID3D11ComputeShader*)shader != DSPtr)
        {
            DSPtr = (ID3D11ComputeShader*)shader;
            context->DSSetShader((ID3D11DomainShader*)shader, nullptr, 0);
            DS = shader != nullptr;
        }
        break;
    }
    case EShaderType::VERTEX_SHADER:
    {
        if ((ID3D11ComputeShader*)shader != VSPtr)
        {
            VSPtr = (ID3D11ComputeShader*)shader;
            context->VSSetShader((ID3D11VertexShader*)shader, nullptr, 0);
            VS = shader != nullptr;
        }
        break;
    }
    case EShaderType::GEOMETRY_SHADER:
    {
        if ((ID3D11ComputeShader*)shader != GSPtr)
        {
            GSPtr = (ID3D11ComputeShader*)shader;
            context->GSSetShader((ID3D11GeometryShader*)shader, nullptr, 0);
            GS = shader != nullptr;
        }
        break;
    }
    }
}

void RenderDeviceDX11::SetupConstBuffers(IConstBufferView* constBuffers[], uint8_t num)
{
    bool flag = num != constBuffersNum;
    for (int i = 0; i < num; i++)
    {
        ID3D11Buffer* buffer = ((ConstBufferViewD3D11**)constBuffers)[i] ? ((ConstBufferViewD3D11**)constBuffers)[i]->constBuffer : nullptr;
        flag |= buffer != constBuffersPtr[i] ;
        constBuffersPtr[i] = buffer;
    }
    for (int i = num; i < 32; i++)
    {
        flag |= nullptr != constBuffersPtr[i] ;
        constBuffersPtr[i] = nullptr;
    }
    if (flag)
    {
        if (CS) context->CSSetConstantBuffers(0, num, constBuffersPtr);
        if (PS) context->PSSetConstantBuffers(0, num, constBuffersPtr);
        if (GS) context->GSSetConstantBuffers(0, num, constBuffersPtr);
        if (DS) context->DSSetConstantBuffers(0, num, constBuffersPtr);
        if (HS) context->HSSetConstantBuffers(0, num, constBuffersPtr);
        if (VS) context->VSSetConstantBuffers(0, num, constBuffersPtr);
    }
}
void RenderDeviceDX11::SetupInputLayout(IInputLayout* layout)
{
    if (layoutPtr != layout)
    {
        layoutPtr = layout;
        context->IASetInputLayout((ID3D11InputLayout*)layout);
    }
}

void RenderDeviceDX11::SetupPipeline(const PipelineDescription& Pipeline)
{
    if (Pipeline.isCS)
    {
        SetupShader(Pipeline.CS, EShaderType::COMPUTE_SHADER);

        HS = false;
        PS = false;
        DS = false;
        VS = false;
        GS = false;
        HSPtr = nullptr;
        PSPtr = nullptr;
        DSPtr = nullptr;
        VSPtr = nullptr;
        GSPtr = nullptr;

        SetupSamplers(Pipeline.samplers, Pipeline.samplersNum);

        
    }
    else
    {
        CS    = false;
        CSPtr = nullptr;
        SetupShader(Pipeline.HS, EShaderType::HULL_SHADER);
        SetupShader(Pipeline.PS, EShaderType::PIXEL_SHADER);
        SetupShader(Pipeline.DS, EShaderType::DOMAIN_SHADER);
        SetupShader(Pipeline.VS, EShaderType::VERTEX_SHADER);
        SetupShader(Pipeline.GS, EShaderType::GEOMETRY_SHADER);
        SetupInputLayout(Pipeline.layout);
        
        SetupViewports(Pipeline.viewports, Pipeline.viewportsNum);
        SetupSamplers(Pipeline.samplers, Pipeline.samplersNum);
        SetupBlendState(Pipeline.blendState);
        SetupDepthStencilState(Pipeline.depthStencilState);
        SetupRasterizerState(Pipeline.rasterizerState);
        SetupPrimitiveTopology(Pipeline.topology);
        
    }
}

void RenderDeviceDX11::ClearState()
{
    context->ClearState();
    CS = false; 
    PS = false; 
    GS = false; 
    DS = false; 
    HS = false; 
    VS = false;
    CSPtr = nullptr;
    PSPtr = nullptr;
    VSPtr = nullptr;
    GSPtr = nullptr;
    HSPtr = nullptr;
    DSPtr = nullptr;


    blendStatePtr = nullptr;;
    blendFactorHash = {};
    depthStencilStatePtr = nullptr;;
    rasterizerStatePtr = nullptr;
    samplersNum = 0;
    d3d11viewportsNum = 0;
    topologyHash =  EPrimitiveTopology::PRIMITIVE_TOPOLOGY_UNKNOWN;

    vertexBuffersNum = 0;
    renderTargetsNum = 0;
    constBuffersNum = 0;
    texturesNum = 0;
    indexBufferPtr = nullptr;
    
}
void RenderDeviceDX11::ClearRenderTarget(IRenderTargetView* rtView, FColor color)
{
    context->ClearRenderTargetView((ID3D11RenderTargetView*)rtView ? (ID3D11RenderTargetView*)rtView : swapchainRTView, color.Color);
}

void RenderDeviceDX11::ClearDepthStencil(IDepthStencilView* dsView, float depth, int8_t stencil)
{
    context->ClearDepthStencilView((ID3D11DepthStencilView*)dsView, D3D11_CLEAR_DEPTH, depth, stencil);
}

void RenderDeviceDX11::ResizeBackbuffer(int32_t width, int32_t height)
{
    backBufferWidth = width;
    backBufferHeight = height;
    
    context->OMSetRenderTargets(0, 0, 0);

    // Release all outstanding references to the swap chain's buffers.
    swapchainRTView->Release();
    swapchainUAView->Release();
    // Preserve the existing buffer count and format.
    // Automatically choose the width and height to match the client rect for HWNDs.
    hr = swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
                                            
    // Perform error handling here!

    // Get buffer and create a render-target-view.
    ID3D11Texture2D* pBuffer;
    hr = swapchain->GetBuffer(0, __uuidof( ID3D11Texture2D),
                                 (void**) &pBuffer );
    // Perform error handling here!

    hr = device->CreateRenderTargetView(pBuffer, NULL,
                                             &swapchainRTView);
    hr = device->CreateUnorderedAccessView(pBuffer, NULL,
                                             &swapchainUAView);
    // Perform error handling here!
    pBuffer->Release();

}

void RenderDeviceDX11::BeginEvent(const char* name)
{
    std::wstring wc(strlen(name), L'#');
    mbstowcs(&wc[0], name, strlen(name));
    perf->BeginEvent(wc.c_str());
}

void RenderDeviceDX11::EndEvent()
{
    perf->EndEvent();
}

void* RenderDeviceDX11::GetNativeTexture(IResourceView* view)
{
    return (void*)view;
}

void RenderDeviceDX11::GetBackbufferSize(uint32_t& w, uint32_t& h)
{
    w = backBufferWidth;
    h = backBufferHeight;
}
void RenderDeviceDX11::SyncBlockExecutionStart() {}
void RenderDeviceDX11::SyncResourcesRead(IResource** data, size_t size) {}
void RenderDeviceDX11::SyncResourcesWrite(IResource** data, size_t size)
{
}
void RenderDeviceDX11::SyncBlockExecutionEnd() {}

void RenderDeviceDX11::Draw(const DrawCall& call)
{
    
        switch (call.type)
        {
        case EDrawCallType::DRAW_INDEXED:
        {
            context->DrawIndexed(call.get<0>(), call.get<1>(), call.get<2>());
            break;
        }
        case EDrawCallType::DRAW:
        {
            context->Draw(call.get<1>(), call.get<0>());
            break;
        }
        case EDrawCallType::DRAW_INDEXED_INSTANCED:
        {
            context->DrawIndexedInstanced(call.get<0>(), call.get<1>(), call.get<2>(), call.get<3>(), call.get<4>());
            break;
        }
        case EDrawCallType::DISPATCH:
        {
            context->Dispatch(call.get<0>(), call.get<1>(), call.get<2>());
            break;
        }
        }
    
}
void RenderDeviceDX11::Present()
{
    GFX_THROW_INFO(swapchain->Present(syncInterval, 0));
}
