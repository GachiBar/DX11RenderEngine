// ReSharper disable CppCStyleCast
#include "D3D11Renderer.h"

#include "D3D11Texture.h"
#include "D3D11Buffer.h"
#include "D3D11Shaders.h"
#include "D3D11RenderBuffer.h"

#include "RenderDeviceDX11.h"


#include "DX11ShadersCompiler.h"
#include "GraphicsAPI.h"
#pragma warning( disable : 26812 )

#pragma comment( lib,"gdiplus.lib" )

using namespace Renderer;

static inline int32_t PrimitiveVerts(
    PrimitiveType primitiveType,
    int32_t primitiveCount
)
{
    switch (primitiveType)
    {
    case PRIMITIVETYPE_TRIANGLELIST: return primitiveCount * 3;
    case PRIMITIVETYPE_TRIANGLESTRIP: return primitiveCount + 2;
    case PRIMITIVETYPE_LINELIST: return primitiveCount * 2;
    case PRIMITIVETYPE_LINESTRIP: return primitiveCount + 1;
    case PRIMITIVETYPE_POINTLIST_EXT: return primitiveCount;
    default: return 0;
    }
}


D3D11Texture D3D11Texture::NullTexture = D3D11Texture
{
    -1,
    0,
    0,
    0,
};


void D3D11Renderer::GetDrawableSize(void* window, int32_t* w, int32_t* h)
{}

void* D3D11Renderer::GetDevice()
{
  return  ((GVM::RenderDeviceDX11*)testRD)->device.Get();
}
void* D3D11Renderer::GetContext()
{
  return  ((GVM::RenderDeviceDX11*)testRD)->context.Get();
}

D3D11Renderer::D3D11Renderer(PresentationParameters presentationParameters, uint8_t debugMode) :
    IRenderer(presentationParameters, debugMode)
{
    auto ppppp = new GVM::Dx11PlatformHandle();
    ppppp->hwnd = (HWND)presentationParameters.deviceWindowHandle;
    GVM::RenderDeviceInitParams init{
        presentationParameters.BackBufferSize.Width,
        presentationParameters.BackBufferSize.Height,
        ppppp,
        GVM::EPresentInterval::PRESENT_INTERVAL_IMMEDIATE
    };

    testRD = new GVM::RenderDeviceDX11(init, debugMode);
    testApi = new GVM::GraphicsApi(testRD);
    delete ppppp;

    shaderCompiler = new DX11ShadersCompiler();
    /* Initialize texture and sampler collections */
    vertexTextures.resize(MAX_VERTEXTEXTURE_SAMPLERS, &D3D11Texture::NullTexture);

    pixelTextures.resize(MAX_TEXTURE_SAMPLERS, &D3D11Texture::NullTexture);

    renderTargetViewsTest.resize(MAX_RENDERTARGET_BINDINGS);

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
    blendFactor.r = 0xFF;
    blendFactor.g = 0xFF;
    blendFactor.b = 0xFF;
    blendFactor.a = 0xFF;
    multiSampleMask = -1; /* AKA 0xFFFFFFFF, ugh -flibit */
    topology = (PrimitiveType)-1; /* Force an update */



    testApi->GetBackBufferSize(mainViewportWidth, mainViewportHeight);;
    testApi->GetBackBufferSize(backBufferWidth, backBufferWidth);;
    /* Set presentation interval */
}

void D3D11Renderer::ResizeBackbuffer(const Size2D& parameters)
{
    backBufferWidth = parameters.Width;
    backBufferHeight = parameters.Height;

    testApi->ResizeBackbuffer(parameters.Width, parameters.Height);
}

void D3D11Renderer::ResizeMainViewport(const Size2D& parameters)
{
    mainViewportWidth = parameters.Width;
    mainViewportHeight = parameters.Height;
}


D3D11Renderer::~D3D11Renderer()
{
    delete testApi;
}

void D3D11Renderer::RunVM()
{
    testApi->RunVM();
}

void D3D11Renderer::SwapBuffers()
{
    testApi->Present();
}

void D3D11Renderer::Clear(ClearOptions options, FColor color, float depth, int32_t stencil)
{
    int32_t i;
    uint32_t dsClearFlags;
    float clearColor[4] = {color.x,color.y,color.z,color.w};

    //std::lock_guard<std::mutex> guard(ctxLock);

    /* Clear color? */
    if (options & ClearOptions::CLEAROPTIONS_TARGET)
    {
        testApi->ClearRenderTargets((renderTargetViewsTest.data()), numRenderTargets, clearColor);
    }

    /* Clear depth/stencil? */
    dsClearFlags = 0;
    if (options & CLEAROPTIONS_DEPTHBUFFER)
    {
        dsClearFlags |= 1;
    }
    if (options & CLEAROPTIONS_STENCIL)
    {
        dsClearFlags |= 2;
    }
    if (dsClearFlags != 0 && depthStencilBuffer != nullptr && depthStencilBuffer->texture != nullptr)
    {
        /* Clear! */
        testApi->ClearDepthStencil(depthStencilBuffer->depth.nDsView, depth, stencil);
    }
}

static GVM::EFormat GVMIndexType[] =
{
    GVM::EFormat::FORMAT_R16_UINT, /* IndexElementSize.SixteenBits */
    GVM::EFormat::FORMAT_R32_UINT /* IndexElementSize.ThirtyTwoBits */
};


GVM::EPrimitiveTopology ToGVM(PrimitiveType type)
{
    switch (type)
    {
    case PRIMITIVETYPE_LINELIST: return GVM::EPrimitiveTopology::PRIMITIVE_TOPOLOGY_LINELIST;
    case PRIMITIVETYPE_POINTLIST_EXT: return GVM::EPrimitiveTopology::PRIMITIVE_TOPOLOGY_POINTLIST;
    case PRIMITIVETYPE_LINESTRIP: return GVM::EPrimitiveTopology::PRIMITIVE_TOPOLOGY_LINESTRIP;
    case PRIMITIVETYPE_TRIANGLELIST: return GVM::EPrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case PRIMITIVETYPE_TRIANGLESTRIP: return GVM::EPrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    }
    return GVM::EPrimitiveTopology::PRIMITIVE_TOPOLOGY_UNKNOWN;
};

void D3D11Renderer::ApplyIndexBufferBinding(const Buffer* indices, uint8_t indexElementSize)
{
    D3D11Buffer* d3dIndices = (D3D11Buffer*)indices;
    /* Bind index buffer */

   if (!d3dIndices)
   {
       testApi->SetupIndexBuffer(nullptr);
       return;
   }
   if (!d3dIndices->indexViewTest)
   {
       d3dIndices->indexViewTest = testApi->CreateIndexBufferView({
           (GVM::IndexBuffer*)d3dIndices->handleTest,GVMIndexType[indexElementSize / 16 - 1],(uint32_t)d3dIndices->size
       });
   }
    


    testApi->SetupIndexBuffer((GVM::IndexBufferView*)d3dIndices->indexViewTest);
}

void D3D11Renderer::DrawIndexedPrimitives(PrimitiveType primitiveType, int32_t baseVertex, int32_t minVertexIndex,
    int32_t numVertices, int32_t startIndex, int32_t primitiveCount)
{
    //std::lock_guard<std::mutex> guard(ctxLock);

    /* Set up draw state */
    if (topology != primitiveType)
    {
        topology = primitiveType;
        testApi->SetupPrimitiveTopology(ToGVM(primitiveType));
    }
    /* Draw! */

    testApi->DrawIndexedPrimitives(PrimitiveVerts(primitiveType, primitiveCount), (uint32_t)startIndex, baseVertex);
}

void D3D11Renderer::DrawInstancedPrimitives(PrimitiveType primitiveType, int32_t baseVertex, int32_t minVertexIndex,
    int32_t numVertices, int32_t startIndex, int32_t primitiveCount,
    int32_t instanceCount)
{
    //std::lock_guard<std::mutex> guard(ctxLock);


    /* Set up draw state */
    if (topology != primitiveType)
    {
        topology = primitiveType;
        testApi->SetupPrimitiveTopology(ToGVM(primitiveType));
    }


    /* Draw! */
    testApi->DrawInstancedPrimitives(baseVertex, 0, PrimitiveVerts(primitiveType, primitiveCount), startIndex,
        primitiveCount, instanceCount);
}

void D3D11Renderer::DrawPrimitives(PrimitiveType primitiveType, int32_t vertexStart, int32_t primitiveCount)
{
    //std::lock_guard<std::mutex> guard(ctxLock);

    /* Bind draw state */
    if (topology != primitiveType)
    {
        topology = primitiveType;
        testApi->SetupPrimitiveTopology(ToGVM(primitiveType));
    }

    /* Draw! */
    testApi->DrawPrimitives(vertexStart, primitiveCount);
}

GVM::ViewportDesc ToGVM(const Viewport& viewport)
{
    GVM::ViewportDesc result;
    result.Height = viewport.h;
    result.Width = viewport.w;
    result.TopLeftX = viewport.x;
    result.TopLeftY = viewport.y;
    result.MinDepth = viewport.minDepth;
    result.MaxDepth = viewport.maxDepth;
    return result;
}

void D3D11Renderer::SetViewport(const Viewport& viewport, uint32_t slot)
{
    testApi->SetupViewport(ToGVM(viewport), slot);
}

void D3D11Renderer::SetBackBufferViewport(const Viewport& viewport)
{
    RenderTargetBinding::BackBufferRT.viewport = viewport;
}

void D3D11Renderer::SetScissorRect(Rect scissor)
{
    //testApi->SetupScissorRect(ToGVM(scissor)); todo
}

void D3D11Renderer::GetBlendFactor(Color& blendFactor)
{
    blendFactor = this->blendFactor;
}

void D3D11Renderer::SetBlendFactor(Color blendFactor)
{
    float factor[4];
    if (this->blendFactor != blendFactor)
    {
        factor[0] = blendFactor.r / 255.0f;
        factor[1] = blendFactor.g / 255.0f;
        factor[2] = blendFactor.b / 255.0f;
        factor[3] = blendFactor.a / 255.0f;
        this->blendFactor = blendFactor;
        //std::lock_guard<std::mutex> guard(ctxLock);

        testApi->SetupCoreBlendState({0xffffffff,{factor[0],factor[1],factor[2],factor[3]}});
    }
}

int32_t D3D11Renderer::GetMultiSampleMask()
{
    return multiSampleMask;
}

void D3D11Renderer::SetMultiSampleMask(int32_t mask)
{
    float factor[4];
    if (multiSampleMask != mask)
    {
        multiSampleMask = mask;
        factor[0] = blendFactor.r / 255.0f;
        factor[1] = blendFactor.g / 255.0f;
        factor[2] = blendFactor.b / 255.0f;
        factor[3] = blendFactor.a / 255.0f;
        //std::lock_guard<std::mutex> guard(ctxLock);
        testApi->SetupCoreBlendState({true,{factor[0],factor[1],factor[2],factor[3]}});
    }
}

constexpr GVM::EBlendType ToGVM(Blend blend)
{
    switch (blend)
    {
    case BLEND_ONE: return GVM::EBlendType::BLEND_ONE;
    case BLEND_ZERO: return GVM::EBlendType::BLEND_ZERO;
    case BLEND_SOURCECOLOR: return GVM::EBlendType::BLEND_SRC_COLOR;
    case BLEND_INVERSESOURCECOLOR: return GVM::EBlendType::BLEND_INV_SRC_COLOR;
    case BLEND_SOURCEALPHA: return GVM::EBlendType::BLEND_SRC_ALPHA;
    case BLEND_INVERSESOURCEALPHA: return GVM::EBlendType::BLEND_INV_SRC_ALPHA;
    case BLEND_DESTINATIONCOLOR: return GVM::EBlendType::BLEND_DEST_COLOR;
    case BLEND_INVERSEDESTINATIONCOLOR: return GVM::EBlendType::BLEND_INV_DEST_COLOR;
    case BLEND_DESTINATIONALPHA: return GVM::EBlendType::BLEND_DEST_ALPHA;
    case BLEND_INVERSEDESTINATIONALPHA: return GVM::EBlendType::BLEND_INV_DEST_ALPHA;
    case BLEND_BLENDFACTOR: return GVM::EBlendType::BLEND_BLEND_FACTOR;
    case BLEND_INVERSEBLENDFACTOR: return GVM::EBlendType::BLEND_INV_BLEND_FACTOR;
    case BLEND_SOURCEALPHASATURATION: return GVM::EBlendType::BLEND_SRC_ALPHA_SAT;
    default: return GVM::EBlendType::BLEND_UNKNOWN;
    }
}

constexpr GVM::EBlendOperator ToGVM(BlendFunction blend)
{
    switch (blend)
    {
    case BLENDFUNCTION_ADD: return GVM::EBlendOperator::BLEND_OP_ADD;
    case BLENDFUNCTION_SUBTRACT: return GVM::EBlendOperator::BLEND_OP_SUBTRACT;
    case BLENDFUNCTION_REVERSESUBTRACT: return GVM::EBlendOperator::BLEND_OP_REV_SUBTRACT;
    case BLENDFUNCTION_MAX: return GVM::EBlendOperator::BLEND_OP_MAX;
    case BLENDFUNCTION_MIN: return GVM::EBlendOperator::BLEND_OP_MIN;
    default: return GVM::EBlendOperator::BLEND_UNKNOWN;
    }
}

GVM::BlendStateDesc ToGVM(const BlendState& blendState)
{
    GVM::BlendStateDesc result;
    result.BlendEnable = blendState.enabled;
    result.BlendOp = ToGVM(blendState.colorBlendFunction);
    result.DestBlend = ToGVM(blendState.colorDestinationBlend);
    result.SrcBlend = ToGVM(blendState.colorSourceBlend);
    result.BlendOpAlpha = ToGVM(blendState.alphaBlendFunction);
    result.DestBlendAlpha = ToGVM(blendState.alphaDestinationBlend);
    result.LogicOpEnable = false;
    result.RenderTargetWriteMask = blendState.colorWriteEnable;
    return result;
}

void D3D11Renderer::SetBlendState(const BlendState& blendState)
{
    float factor[4];

    auto gvmBS = ToGVM(blendState);
    testApi->SetupBlendState(gvmBS, 0);
    testApi->SetupBlendState(gvmBS, 1);
    testApi->SetupBlendState(gvmBS, 2);
    testApi->SetupBlendState(gvmBS, 3);
    testApi->SetupBlendState(gvmBS, 4);
    testApi->SetupBlendState(gvmBS, 5);
    testApi->SetupBlendState(gvmBS, 6);
    testApi->SetupBlendState(gvmBS, 7);

    //if (renderer->blendState != bs ||
    //	!D3D11_INTERNAL_BlendEquals(&renderer->blendFactor, &blendState->blendFactor) ||
    //	renderer->multiSampleMask != blendState->multiSampleMask) 
    {
        factor[0] = blendState.blendFactor.r / 255.0f;
        factor[1] = blendState.blendFactor.g / 255.0f;
        factor[2] = blendState.blendFactor.b / 255.0f;
        factor[3] = blendState.blendFactor.a / 255.0f;
        blendFactor = blendState.blendFactor;
        multiSampleMask = blendState.multiSampleMask;
        //std::lock_guard<std::mutex> guard(ctxLock);
        testApi->SetupCoreBlendState({true,{factor[0],factor[1],factor[2],factor[3]}});
    }
}

constexpr GVM::EComparisonFunc ToGVM(CompareFunction func)
{
    switch (func)
    {
    case COMPAREFUNCTION_ALWAYS: return GVM::EComparisonFunc::COMPARISON_ALWAYS;
    case COMPAREFUNCTION_NEVER: return GVM::EComparisonFunc::COMPARISON_NEVER;
    case COMPAREFUNCTION_LESS: return GVM::EComparisonFunc::COMPARISON_LESS;
    case COMPAREFUNCTION_LESSEQUAL: return GVM::EComparisonFunc::COMPARISON_LESS_EQUAL;
    case COMPAREFUNCTION_EQUAL: return GVM::EComparisonFunc::COMPARISON_EQUAL;
    case COMPAREFUNCTION_GREATEREQUAL: return GVM::EComparisonFunc::COMPARISON_GREATER_EQUAL;
    case COMPAREFUNCTION_GREATER: return GVM::EComparisonFunc::COMPARISON_GREATER;
    case COMPAREFUNCTION_NOTEQUAL: return GVM::EComparisonFunc::COMPARISON_NOT_EQUAL;
    default: return GVM::EComparisonFunc::COMPARISON_UNKNOWN;
    }
}

GVM::DepthStencilStateDesc ToGVM(const DepthStencilState& dsState)
{
    GVM::DepthStencilStateDesc result;
    result.DepthEnable = dsState.depthBufferEnable;
    result.FrontFace.StencilFunc = ToGVM(dsState.stencilFunction);
    result.FrontFace.StencilFunc = ToGVM(dsState.stencilFunction);
    result.FrontFace.StencilFunc = ToGVM(dsState.stencilFunction);

    //todo
    return result;
}


void D3D11Renderer::SetDepthStencilState(const DepthStencilState& depthStencilState)
{
    testApi->SetupDepthStencilState(ToGVM(depthStencilState));
}

constexpr GVM::ECullMode ToGVM(CullMode mode)
{
    switch (mode)
    {
    case CULLMODE_NONE: return GVM::ECullMode::CULL_NONE;
    case CULLMODE_CULLCLOCKWISEFACE: return GVM::ECullMode::CULL_BACK;
    case CULLMODE_CULLCOUNTERCLOCKWISEFACE: return GVM::ECullMode::CULL_FRONT;
    }
    return GVM::ECullMode::CULL_UNKNOWN;
};


constexpr GVM::EFillMode ToGVM(FillMode mode)
{
    switch (mode)
    {
    case FILLMODE_SOLID: return GVM::EFillMode::FILL_SOLID;
    case FILLMODE_WIREFRAME: return GVM::EFillMode::FILL_WIREFRAME;
    }
    return GVM::EFillMode::FILL_UNKNOWN;
}

GVM::RasterizerStateDesc ToGVM(const RasterizerState& rasterizerState)
{
    GVM::RasterizerStateDesc result;

    result.ScissorEnable = rasterizerState.scissorTestEnable;
    result.CullMode = ToGVM(rasterizerState.cullMode);
    result.FillMode = ToGVM(rasterizerState.fillMode);
    result.FrontCounterClockwise = 1;
    return result;
}

void D3D11Renderer::ApplyRasterizerState(const RasterizerState& rasterizerState)
{
    testApi->SetupRasterizerState(ToGVM(rasterizerState));
}


void D3D11Renderer::VerifyPixelTexture(int32_t index, const Texture* texture)
{
    D3D11Texture* d3dTexture = (D3D11Texture*)texture;
    if (texture == nullptr)
    {
        testApi->SetupTexture(nullptr, index);
        return;
    }

    /* Bind the correct texture */
    if (!pixelTextures[index] || d3dTexture != pixelTextures[index])
    {
        pixelTextures[index] = d3dTexture;
        testApi->SetupTexture(d3dTexture->shView, index);
    }
}

void D3D11Renderer::VerifyUATexture(int32_t index, const Texture* texture)
{
    if (index < MAX_RENDERTARGET_BINDINGS)
    {
        D3D11Texture* d3dTexture = (D3D11Texture*)texture;
        if (texture == nullptr)
        {
            testApi->SetupUATexture(nullptr, index);
            return;
        }
        testApi->SetupUATexture(d3dTexture->uaTestView, index);
    }
}

constexpr GVM::ESamplerFilter ToGVM(TextureFilter filter)
{
    switch (filter)
    {
    case TEXTUREFILTER_LINEAR: return GVM::ESamplerFilter::FILTER_MIN_MAG_MIP_LINEAR;
    case TEXTUREFILTER_POINT: return GVM::ESamplerFilter::FILTER_MIN_MAG_MIP_POINT;
    case TEXTUREFILTER_ANISOTROPIC: return GVM::ESamplerFilter::FILTER_ANISOTROPIC;
    case TEXTUREFILTER_LINEAR_MIPPOINT: return GVM::ESamplerFilter::FILTER_MIN_MAG_LINEAR_MIP_POINT;
    case TEXTUREFILTER_POINT_MIPLINEAR: return GVM::ESamplerFilter::FILTER_MIN_MAG_POINT_MIP_LINEAR;
    case TEXTUREFILTER_MINLINEAR_MAGPOINT_MIPLINEAR: return GVM::ESamplerFilter::FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    case TEXTUREFILTER_MINLINEAR_MAGPOINT_MIPPOINT: return GVM::ESamplerFilter::FILTER_MIN_LINEAR_MAG_MIP_POINT;
    case TEXTUREFILTER_MINPOINT_MAGLINEAR_MIPLINEAR: return GVM::ESamplerFilter::FILTER_MIN_POINT_MAG_MIP_LINEAR;
    case TEXTUREFILTER_MINPOINT_MAGLINEAR_MIPPOINT: return GVM::ESamplerFilter::FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
    }
    return GVM::ESamplerFilter::FILTER_UNKNOWN;
}

constexpr GVM::ETextureAddressMode ToGVM(TextureAddressMode mode)
{
    switch (mode)
    {
    case TEXTUREADDRESSMODE_WRAP: return GVM::ETextureAddressMode::TEXTURE_ADDRESS_WRAP;
    case TEXTUREADDRESSMODE_CLAMP: return GVM::ETextureAddressMode::TEXTURE_ADDRESS_CLAMP;
    case TEXTUREADDRESSMODE_MIRROR: return GVM::ETextureAddressMode::TEXTURE_ADDRESS_MIRROR;
    }
    return GVM::ETextureAddressMode::TEXTURE_ADDRESS_UNKNOWN;
}


GVM::SamplerStateDesc ToGVM(const SamplerState& sampler)
{
    GVM::SamplerStateDesc result;

    result.Filter = ToGVM(sampler.filter);
    result.AddressU = ToGVM(sampler.addressU);
    result.AddressV = ToGVM(sampler.addressV);
    result.AddressW = ToGVM(sampler.addressW);
    result.MipLODBias = 0;
    result.MaxAnisotropy = sampler.maxAnisotropy;
    result.ComparisonFunc = GVM::EComparisonFunc::COMPARISON_UNKNOWN;
    result.MinLOD = sampler.minMipLevel;
    result.MaxLOD = sampler.maxMipLevel;

    return result;
}

void D3D11Renderer::VerifyPixelSampler(int32_t index, const SamplerState& sampler)
{
    if (index < MAX_TEXTURE_SAMPLERS)
    {
        /* Update the sampler state, if needed */
        testApi->SetupSampler(ToGVM(sampler), index);
    }
}


void D3D11Renderer::VerifyVertexTexture(int32_t index, const Texture* texture)
{
    D3D11Texture* d3dTexture = (D3D11Texture*)texture;

    if (d3dTexture->levelCount == -1)
    {
        return;
    }

    /* Bind the correct texture */
    if (d3dTexture != vertexTextures[index])
    {
        vertexTextures[index] = d3dTexture;
        testApi->SetupTexture(d3dTexture->shView, index);
    }
}



void D3D11Renderer::VerifyVertexSampler(int32_t index, const SamplerState& sampler)
{
    /* Update the sampler state, if needed */
    //if (d3dSamplerState != renderer->samplers[index]) 

    testApi->SetupSampler(ToGVM(sampler), index);
}

RenderTargetBinding RenderTargetBinding::BackBufferRT{
    1,
    0,
    nullptr,
    nullptr,
    {}

};

void D3D11Renderer::SetRenderTargets(RenderTargetBinding** renderTargets, int32_t numRenderTargets,
    Renderbuffer* depthStencilBuffer)
{
    D3D11Texture* tex;
    D3D11Renderbuffer* rb;
    this->depthStencilBuffer = (D3D11Renderbuffer*)depthStencilBuffer;
    static GVM::RenderTargetView* nviews[MAX_RENDERTARGET_BINDINGS];
    int32_t i;
    if (numRenderTargets == 0)
    {
        nviews[0] = nullptr;
        testApi->SetupRenderTargets(nviews, 0, 0, nullptr);
        testApi->SetupNumRenderTargets(1);
        return;
    }

    /* Update color buffers */
    for (i = 0; i < std::min(numRenderTargets, MAX_RENDERTARGET_BINDINGS); i += 1)
    {
        if (renderTargets[i] == &RenderTargetBinding::BackBufferRT)
        {
            nviews[i] = nullptr;
            renderTargetViewsTest[i] = nullptr;
            this->SetViewport(RenderTargetBinding::BackBufferRT.viewport, i);
            testApi->SetupViewport(ToGVM(RenderTargetBinding::BackBufferRT.viewport), i);
        }
        else
        {
            if (renderTargets[i]->colorBuffer != NULL)
            {
                rb = (D3D11Renderbuffer*)renderTargets[i]->colorBuffer;
                nviews[i] = rb->color.nRsView;
                renderTargetViewsTest[i] = rb->color.nRsView;
            }
            else
            {
                tex = (D3D11Texture*)renderTargets[i]->texture;

                //if (tex->rtType == FNA3D_RENDERTARGET_TYPE_2D) {
                //	views[i] = tex->twod.rtView;
                //} else if (tex->rtType == FNA3D_RENDERTARGET_TYPE_CUBE) {
                //	views[i] = tex->cube.rtViews[
                //		renderTargets[i].cube.face
                //	];
                //}
                nviews[i] = tex->rtTestView;
                renderTargetViewsTest[i] = tex->rtTestView;
            }
            this->SetViewport(renderTargets[i]->viewport, i);
            testApi->SetupViewport(ToGVM(renderTargets[i]->viewport), i);
        }
    }
    while (i < MAX_RENDERTARGET_BINDINGS)
    {
        renderTargetViewsTest[i] = nullptr;
        i++;
    }


    /* Actually set the render targets, finally. */
    //std::lock_guard<std::mutex> guard(ctxLock);
    //DiscardTargetTextures(views, numRenderTargets);
    if (depthStencilBuffer == nullptr)
    {
        depthStencilBufferTest = nullptr;
    }
    else
    {
        depthStencilBufferTest = this->depthStencilBuffer->depth.nDsView;
    }

    testApi->SetupRenderTargets(nviews, numRenderTargets, 0, depthStencilBufferTest);
    //RestoreTargetTextures();


    /* Remember color attachments */
    this->numRenderTargets = numRenderTargets;
    
}

void D3D11Renderer::ResolveTarget(const RenderTargetBinding& target)
{ }

void D3D11Renderer::ReadBackbuffer(int32_t x, int32_t y, int32_t w, int32_t h, void* data, int32_t dataLength)
{}

void D3D11Renderer::GetBackbufferSize(uint32_t& w, uint32_t& h)
{
    testApi->GetBackBufferSize(w, h);
}

void D3D11Renderer::GetMainViewportSize(uint32_t& w, uint32_t& h)
{
    w = static_cast<int32_t>(mainViewportWidth);
    h = static_cast<int32_t>(mainViewportHeight);
}

DepthFormat D3D11Renderer::GetBackbufferDepthFormat()
{
    return this->depthStencilBuffer->depth.format;
}

constexpr GVM::EFormat ToGVM(SurfaceFormat format)
{
    switch (format)
    {
    case SURFACEFORMAT_COLOR: return GVM::EFormat::FORMAT_R8G8B8A8_UNORM;
    case SURFACEFORMAT_BGR565: return GVM::EFormat::FORMAT_UNKNOWN;     //todo
    case SURFACEFORMAT_BGRA5551: return GVM::EFormat::FORMAT_UNKNOWN;   //todo
    case SURFACEFORMAT_BGRA4444: return GVM::EFormat::FORMAT_UNKNOWN;   //todo
    case SURFACEFORMAT_DXT1: return GVM::EFormat::FORMAT_UNKNOWN;       //todo
    case SURFACEFORMAT_DXT3: return GVM::EFormat::FORMAT_UNKNOWN;       //todo
    case SURFACEFORMAT_DXT5: return GVM::EFormat::FORMAT_UNKNOWN;       //todo
    case SURFACEFORMAT_NORMALIZEDBYTE2: return GVM::EFormat::FORMAT_R8G8_SNORM;
    case SURFACEFORMAT_NORMALIZEDBYTE4: return GVM::EFormat::FORMAT_R8G8B8A8_SNORM;
    case SURFACEFORMAT_RGBA1010102: return GVM::EFormat::FORMAT_R10G10B10A2_UNORM;
    case SURFACEFORMAT_RG32: return GVM::EFormat::FORMAT_R16G16_UNORM;
    case SURFACEFORMAT_RGBA64: return GVM::EFormat::FORMAT_R16G16B16A16_UNORM;
    case SURFACEFORMAT_ALPHA8: return GVM::EFormat::FORMAT_A8_UNORM;
    case SURFACEFORMAT_SINGLE: return GVM::EFormat::FORMAT_R32_FLOAT;
    case SURFACEFORMAT_VECTOR2: return GVM::EFormat::FORMAT_R32G32_FLOAT;
    case SURFACEFORMAT_UINT: return GVM::EFormat::FORMAT_R32_UINT;
    case SURFACEFORMAT_VECTOR4: return GVM::EFormat::FORMAT_R32G32B32A32_FLOAT;
    case SURFACEFORMAT_HALFSINGLE: return GVM::EFormat::FORMAT_R16_FLOAT;
    case SURFACEFORMAT_HALFVECTOR2: return GVM::EFormat::FORMAT_R16G16_FLOAT;
    case SURFACEFORMAT_HALFVECTOR4: return GVM::EFormat::FORMAT_R16G16B16A16_FLOAT;
    case SURFACEFORMAT_HDRBLENDABLE: return GVM::EFormat::FORMAT_R16G16B16A16_FLOAT;
    case SURFACEFORMAT_COLORBGRA_EXT: return GVM::EFormat::FORMAT_B8G8R8A8_UNORM;
    case SURFACEFORMAT_COLORSRGB_EXT: return GVM::EFormat::FORMAT_R8G8B8A8_UNORM_SRGB;
    case SURFACEFORMAT_DXT5SRGB_EXT: return GVM::EFormat::FORMAT_UNKNOWN; //todo
    case SURFACEFORMAT_BC7_EXT: return GVM::EFormat::FORMAT_UNKNOWN; //todo
    case SURFACEFORMAT_BC7SRGB_EXT: return GVM::EFormat::FORMAT_UNKNOWN; //todo
    case SURFACEFORMAT_VECTOR3: return GVM::EFormat::FORMAT_R32G32B32_FLOAT; //todo
    }
}

Texture* D3D11Renderer::CreateTexture2D(SurfaceFormat format, int32_t width, int32_t height, int32_t levelCount,
    uint8_t isRenderTarget)
{
    D3D11Texture* result = new D3D11Texture();


    GVM::Texture2DResourceDesc resourceDesc;

    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.Format = ToGVM(format);
    resourceDesc.Array = 1;
    resourceDesc.initialData = nullptr;

    result->resource = testApi->CreateTexture2D(resourceDesc);

    /* Initialize D3D11Texture */
    result->levelCount = levelCount;
    result->isRenderTarget = isRenderTarget;
    result->width = width;
    result->height = height;
    result->format = format;


    /* Create the shader resource view */
    result->shView = testApi->CreateShaderResourceView({result->resource,true});

    /* Create the render target view, if applicable */
    if (isRenderTarget)
    {
        result->isRenderTarget = 1;
        result->rtTestView = testApi->CreateRtView({result->resource,true});
    }

    return result;
}

Texture* D3D11Renderer::CreateUATexture2D(SurfaceFormat format, int32_t width, int32_t height, int32_t levelCount)
{
    D3D11Texture* result = new D3D11Texture();

    GVM::Texture2DResourceDesc resourceDesc;

    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.Format = ToGVM(format);
    resourceDesc.Array = 1;
    resourceDesc.initialData = nullptr;

    result->resource = testApi->CreateTexture2D(resourceDesc);

    /* Initialize D3D11Texture */
    result->levelCount = levelCount;
    result->isRenderTarget = true;
    result->width = width;
    result->height = height;
    result->format = format;

    result->shView = testApi->CreateShaderResourceView({result->resource,true});

    /* Create the render target view, if applicable */

    result->isRenderTarget = 1;
    result->rtTestView = testApi->CreateRtView({result->resource,true});

    GVM::UATargetViewDesc res = GVM::UATargetViewDesc{result->resource,true};
    result->uaTestView = testApi->CreateUaView(res);

    return result;
}


Texture* D3D11Renderer::CreateTextureCube(SurfaceFormat format, int32_t size, int32_t levelCount,
    uint8_t isRenderTarget)
{
    D3D11Texture* result;



    GVM::TextureCubeResourceDesc resourceDesc;

    resourceDesc.Width = size;
    resourceDesc.Height = size;
    resourceDesc.Format = ToGVM(format);
    resourceDesc.Array = 6;
    resourceDesc.initialData = nullptr;

    //ERROR_CHECK_RETURN("TextureCube creation failed", NULL)

    /* Initialize D3D11Texture */
    result = new D3D11Texture();
    //SDL_memset(result, '\0', sizeof(D3D11Texture));
    result->levelCount = levelCount;
    result->isRenderTarget = isRenderTarget;
    result->format = format;

    result->resource = testApi->CreateTextureCube(resourceDesc);

    /* Create the shader resource view */

    result->shView = testApi->CreateShaderResourceView({result->resource,true});
    //todo fix it



    return result;
}

void D3D11Renderer::SetTextureDataCube(Texture* texture, int32_t x, int32_t y, int32_t w, int32_t h,
    int32_t cubeMapFace, int32_t level, void* data, int32_t dataLength)
{
    D3D11Texture* d3dTexture = (D3D11Texture*)texture;

    //int32_t blockSize = Texture_GetBlockSize(d3dTexture->format);

    ///if (blockSize > 1) {
    //	w = (w + blockSize - 1) & ~(blockSize - 1);
    //	h = (h + blockSize - 1) & ~(blockSize - 1);
    //}


    uint8_t byteLength = GVM::FormatByteSize[GVM::to_underlying(ToGVM(d3dTexture->format))];

    testApi->SetResourceData(d3dTexture->resource,
        cubeMapFace,
        GVM::UBox{
            x,
            y,
            0,
            x + w,
            y + h,
            1
        }, data, w * byteLength, byteLength * h * w);

    //SDL_LockMutex(renderer->ctxLock);
}

void* Renderer::D3D11Renderer::GetNativeTexture(Texture* texture)
{
    return testApi->GetNativeTexture(((D3D11Texture*)texture)->shView);
}


void D3D11Renderer::AddDisposeTexture(Texture* texture)
{
    D3D11Texture* tex = (D3D11Texture*)texture;
    if (texture == nullptr)
        return;

    /* Unbind the texture */
    for (int i = 0; i < MAX_TEXTURE_SAMPLERS; i += 1)
    {
        if (pixelTextures[i] && pixelTextures[i] == tex)
        {
            pixelTextures[i] = &D3D11Texture::NullTexture;
        }
    }

    /* Release the shader resource view and texture */
    //tex->shaderView->Release();
    //tex->handle->Release();
    testApi->AddDisposeResource(tex->resource);
    delete tex;
}

void D3D11Renderer::SetTextureData2D(Texture* texture, int32_t x, int32_t y, int32_t w, int32_t h, int32_t level,
    void* data, int32_t dataLength)
{
    D3D11Texture* d3dTexture = (D3D11Texture*)texture;


    uint8_t byteLength = GVM::FormatByteSize[GVM::to_underlying(ToGVM(d3dTexture->format))];


    testApi->SetResourceData(d3dTexture->resource,
        level,
        GVM::UBox{
            x,
            y,
            0,
            x + w,
            y + h,
            1
        }, data, w * byteLength,
        w * byteLength * h);
}


#pragma warning(push)
#pragma warning(disable : 26451)
void D3D11Renderer::GetTextureData2D(const Texture* texture, int32_t x, int32_t y, int32_t w, int32_t h,
    int32_t level, void* data, int32_t dataLength)
{
    D3D11Texture* tex = (D3D11Texture*)texture;
    uint32_t subresourceIndex = level;
    unsigned int texW = static_cast<unsigned int>(tex->width >> level);
    unsigned int texH = static_cast<unsigned int>(tex->height >> level);
    uint8_t* dataPtr = (uint8_t*)data;

    int32_t formatSize = 4;
}

#pragma warning(pop)


Renderbuffer* D3D11Renderer::GenColorRenderbuffer(int32_t width, int32_t height, SurfaceFormat format,
    int32_t multiSampleCount, Texture* texture)
{
    return nullptr;
}


GVM::EDepthFormat ToGVM(DepthFormat format)
{
    switch (format)
    {
    case DEPTHFORMAT_D16: return GVM::EDepthFormat::FORMAT_D16_UNORM;
    case DEPTHFORMAT_D24:
    case DEPTHFORMAT_D24S8: return GVM::EDepthFormat::FORMAT_D24_UNORM_S8_UINT;
    case DEPTHFORMAT_D32: return GVM::EDepthFormat::FORMAT_D32_UNORM;
    default: return GVM::EDepthFormat::FORMAT_UNKNOWN;
    }
}


GVM::EFormat ToGVM(GVM::EDepthFormat format)
{
    switch (format)
    {
    case GVM::EDepthFormat::FORMAT_D16_UNORM: return GVM::EFormat::FORMAT_R16_TYPELESS;
    case GVM::EDepthFormat::FORMAT_D24:
    case GVM::EDepthFormat::FORMAT_D24_UNORM_S8_UINT: return GVM::EFormat::FORMAT_R32_TYPELESS;
    case GVM::EDepthFormat::FORMAT_D32_UNORM: return GVM::EFormat::FORMAT_R32_TYPELESS;
    default: return GVM::EFormat::FORMAT_UNKNOWN;
    }
}

GVM::EFormat ToGVMSh(DepthFormat format)
{
    switch (format)
    {
    case DEPTHFORMAT_D16: return GVM::EFormat::FORMAT_R16_UNORM;
    case DEPTHFORMAT_D24:
    case DEPTHFORMAT_D24S8:
    case DEPTHFORMAT_D32: return GVM::EFormat::FORMAT_R32_FLOAT;
    default: return GVM::EFormat::FORMAT_UNKNOWN;
    }
}

Renderbuffer* D3D11Renderer::GenDepthStencilRenderbuffer(int32_t width, int32_t height, DepthFormat format,
    int32_t multiSampleCount)
{
    D3D11Renderbuffer* result = new D3D11Renderbuffer(RENDERBUFFER_DEPTH);
    /* Initialize the renderbuffer */
    result->multiSampleCount = multiSampleCount;
    result->depth.format = format;
    D3D11Texture* texture = new D3D11Texture();

    GVM::Texture2DResourceDesc resourceDesc;

    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.Format = ToGVM(ToGVM(format));
    resourceDesc.Array = 1;
    resourceDesc.initialData = nullptr;

    result->resource = testApi->CreateTexture2D(resourceDesc);

    texture->isRenderTarget = true;

    GVM::ShaderResourceViewDesc shDesc;
    shDesc.Dimension = GVM::EShaderViewDimension::DIMENSION_TEXTURE2D;
    shDesc.Format = ToGVMSh(format);
    shDesc.Resource = result->resource;
    shDesc.T2Desc.PlaneSlice = 0;
    shDesc.T2Desc.ResourceMinLODClamp = 0;
    result->depth.nShView = testApi->CreateShaderResourceView(shDesc);

    texture->shView =  result->depth.nShView;
    

    result->texture = texture;


    GVM::DepthStencilViewDesc view_desc;
    view_desc.Resource = result->resource;
    view_desc.Flag = GVM::EDsvFlags::DSV_FLAG_NONE;
    view_desc.Format = ToGVM(format);
    view_desc.Resource = result->resource;


    result->depth.nDsView = testApi->CreateDepthStencilsView(view_desc);

    return result;
}

void D3D11Renderer::AddDisposeRenderbuffer(Renderbuffer* renderbuffer)
{
    D3D11Renderbuffer* d3dRenderbuffer = (D3D11Renderbuffer*)renderbuffer;
    D3D11Texture* d3dTexture = (D3D11Texture*)(renderbuffer->texture);
    AddDisposeTexture(d3dTexture);

    testApi->AddDisposeResource(d3dRenderbuffer->resource);
    delete d3dRenderbuffer;
}

Buffer* D3D11Renderer::GenVertexBuffer(uint8_t dynamic, BufferUsage usage, int32_t sizeInBytes)
{
    D3D11Buffer* result = new D3D11Buffer();


    GVM::BufferResourceDesc tdesc;
    tdesc.Size = sizeInBytes;
    result->handleTest = testApi->CreateBuffer(tdesc);
    /* Return the result */
    result->dynamic = dynamic;
    return result;
}

void D3D11Renderer::AddDisposeVertexBuffer(Buffer* buffer)
{
    D3D11Buffer* d3dBuffer = (D3D11Buffer*)buffer;
    uint32_t whatever[1] = {0};

    testApi->AddDisposeResource(d3dBuffer->handleTest);
    delete d3dBuffer;
}

void D3D11Renderer::SetVertexBufferData(Buffer* buffer, int32_t offsetInBytes, void* data, int32_t elementCount,
    int32_t elementSizeInBytes, int32_t vertexStride, SetDataOptions options)
{
    D3D11Buffer* d3dBuffer = (D3D11Buffer*)buffer;
    int32_t dataLen = vertexStride * elementCount;

    testApi->SetVertexBufferData((GVM::VertexBuffer*)d3dBuffer->handleTest, data, dataLen, offsetInBytes);
}

void D3D11Renderer::GetVertexBufferData(const Buffer* buffer, int32_t offsetInBytes, void* data,
    int32_t elementCount, int32_t elementSizeInBytes, int32_t vertexStride)
{ }

Buffer* D3D11Renderer::GenIndexBuffer(uint8_t dynamic, BufferUsage usage, int32_t sizeInBytes)
{
    D3D11Buffer* result = new D3D11Buffer();


    GVM::BufferResourceDesc tdesc;
    tdesc.Size = sizeInBytes;
    result->handleTest = testApi->CreateBuffer(tdesc);

    /* Return the result */
    result->dynamic = dynamic;
    return result;
}

void D3D11Renderer::AddDisposeIndexBuffer(Buffer* buffer)
{
    D3D11Buffer* d3dBuffer = (D3D11Buffer*)buffer;

    testApi->AddDisposeResource(d3dBuffer->handleTest);
    delete d3dBuffer;
}

void D3D11Renderer::SetIndexBufferData(Buffer* buffer, int32_t offsetInBytes, void* data, int32_t dataLength,
    SetDataOptions options)
{
    D3D11Buffer* d3dBuffer = (D3D11Buffer*)buffer;
    testApi->SetIndexBufferData((GVM::IndexBuffer*)d3dBuffer->handleTest, data, dataLength, offsetInBytes);
}

void D3D11Renderer::GetIndexBufferData(const Buffer* buffer, int32_t offsetInBytes, void* data, int32_t dataLength)
{ }


void D3D11Renderer::ResetBackbuffer(const PresentationParameters& presentationParameters)
{ }




void D3D11Renderer::ApplyVertexBufferBinding(const VertexBufferBinding& vertexBuffer)
{
    static GVM::VertexBufferBinding vb;
    if (vertexBuffer.buffersCount == 1)
    {
        //std::lock_guard<std::mutex> guard(ctxLock);
        auto buff = (D3D11Buffer*)(vertexBuffer.vertexBuffers[0]);
        vb.buffersNum = 1;
        if (!buff->vertexViewTest)
            buff->vertexViewTest = testApi->CreateVertexBufferView({
                buff->vertexTest,
                uint32_t(vertexBuffer.vertexStride[0]),
                vertexBuffer.vertexOffset[0]
            });
        vb.vertexBuffers[0] = buff->vertexViewTest;
        //vb.vertexStride[0] = vertexBuffer.vertexStride[0];
        //vb.vertexOffset[0] = vertexBuffer.vertexOffset[0];
    }
    else
    {
        vb.buffersNum = vertexBuffer.buffersCount;
        for (size_t i = 0; i < vertexBuffer.buffersCount; i++)
        {
            auto* buff = ((D3D11Buffer*)(vertexBuffer.vertexBuffers)[i]);

            if (!buff->vertexViewTest)
                buff->vertexViewTest = testApi->CreateVertexBufferView({
                    buff->vertexTest,
                    uint32_t(vertexBuffer.vertexStride[i]),
                    vertexBuffer.vertexOffset[i]
                });
            vb.vertexBuffers[i] = buff->vertexViewTest;
            //vb.vertexStride[0] = vertexBuffer.vertexStride[0];
            //vb.vertexOffset[0] = vertexBuffer.vertexOffset[0];
        }
        // this->vertexBuffer = buffers[0];
        //std::lock_guard<std::mutex> guard(ctxLock);
    }
    testApi->SetupVertexBuffer(vb);
}

PixelShader* D3D11Renderer::CompilePixelShader(const ShaderCompileData& shaderData)
{
    auto result = shaderCompiler->CompilePixelShader(shaderData);

    GVM::ShaderDesc desc;
    desc.type = GVM::EShaderType::PIXEL_SHADER;
#ifdef _DEBUG
    desc.name = shaderData.name;
#endif // _DEBUG
    desc.bytecode = result->data;
    desc.byteCodeSize = result->dataSize;
    result->testShader = testApi->CreateShader(desc);

    return result;
}


ComputeShader* D3D11Renderer::CompileComputeShader(const ShaderCompileData& shaderData)
{
    auto result = shaderCompiler->CompileComputeShader(shaderData);

    GVM::ShaderDesc desc;
    desc.type = GVM::EShaderType::COMPUTE_SHADER;
#ifdef _DEBUG
    desc.name = shaderData.name;
#endif // _DEBUG
    desc.bytecode = result->data;
    desc.byteCodeSize = result->dataSize;
    result->testShader = testApi->CreateShader(desc);

    return result;
}

GeometryShader* D3D11Renderer::CompileGeometryShader(const ShaderCompileData& shaderData)
{
    auto result = shaderCompiler->CompileGeometryShader(shaderData);

    GVM::ShaderDesc desc;
    desc.type = GVM::EShaderType::GEOMETRY_SHADER;

#ifdef _DEBUG
    desc.name = shaderData.name;
#endif // _DEBUG
    desc.bytecode = result->data;
    desc.byteCodeSize = result->dataSize;
    result->testShader = testApi->CreateShader(desc);

    return result;
}



VertexShader* D3D11Renderer::CompileVertexShader(const ShaderCompileData& shaderData, void* inputLayout,
    size_t inputLayoutSize)
{
    auto result = shaderCompiler->CompileVertexShader(shaderData);

    GVM::ShaderDesc desc;
    desc.type = GVM::EShaderType::VERTEX_SHADER;
#ifdef _DEBUG
        desc.name = shaderData.name;
#endif // _DEBUG
    desc.bytecode = result->data;
    desc.byteCodeSize = result->dataSize;
    result->testShader = testApi->CreateShader(desc);
    result->testIL = testApi->CreateInputLayout(shaderCompiler->ToGVM(inputLayout, inputLayoutSize), desc);
    return result;
}



void D3D11Renderer::AddDisposePixelShader(PixelShader* pixelShader)
{
    D3D11PixelShader* shader = (D3D11PixelShader*)pixelShader;
    if (shader == nullptr)
        return;
    delete (char*)shader->data;
    testApi->AddDisposeShader(shader->testShader);
    delete shader;
}

void D3D11Renderer::AddDisposeVertexShader(VertexShader* vertexShader)
{
    D3D11VertexShader* shader = (D3D11VertexShader*)vertexShader;
    if (shader == nullptr)
        return;
    delete (char*)shader->data;
    testApi->AddDisposeShader(shader->testShader);
    testApi->AddDisposeIL(shader->testIL);
    delete shader;
}

void D3D11Renderer::ApplyPixelShader(PixelShader* pixelShader)
{
    if (!pixelShader)
        return;
    D3D11PixelShader* shader = (D3D11PixelShader*)pixelShader;
    testApi->SetupShader(shader->testShader, GVM::EShaderType::PIXEL_SHADER);
}

void D3D11Renderer::ApplyVertexShader(VertexShader* vertexShader)
{
    if (!vertexShader)
        return;
    D3D11VertexShader* shader = (D3D11VertexShader*)vertexShader;
    testApi->SetupInputLayout(shader->testIL);
    testApi->SetupShader(shader->testShader, GVM::EShaderType::VERTEX_SHADER);
}

ConstBuffer* D3D11Renderer::CreateConstBuffer(size_t size)
{
    D3D11ConstBuffer* result = new D3D11ConstBuffer();

    //cbd.Usage = D3D11_USAGE_DYNAMIC;
    //cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    GVM::BufferResourceDesc rDesc;
    rDesc.initialData = nullptr;
    rDesc.Size = ((size / 16 + (size % 16 != 0)) * 16);


    result->buffer = testApi->CreateConstBuffer(rDesc);
    result->bufferView = testApi->CreateConstBufferView({result->buffer,rDesc.Size});

    result->size = size;

    return result;
}

void D3D11Renderer::VerifyConstBuffer(ConstBuffer* constBuffer, size_t slot)
{
    D3D11ConstBuffer* buff = (D3D11ConstBuffer*)constBuffer;
    //std::lock_guard<std::mutex> guard(ctxLock);
    testApi->SetupConstBuffer(buff->bufferView, slot);
}

void D3D11Renderer::SetConstBuffer(ConstBuffer* constBuffers, void* data)
{
    D3D11ConstBuffer* buffer = (D3D11ConstBuffer*)constBuffers;

    testApi->SetConstBufferData(buffer->buffer, data, buffer->size);

    //GFX_THROW_INFO_ONLY(context->Map(
    //      buffer->handle.Get(),
    //      0,
    //      D3D11_MAP_WRITE_DISCARD,
    //      0,
    //      &subres
    //  ));
    //memcpy(
    //    (uint8_t*)subres.pData,
    //    data,
    //    buffer->size
    //);
    //GFX_THROW_INFO_ONLY(context->Unmap(
    //    buffer->handle.Get(),
    //    0
    //));
}

void D3D11Renderer::AddDisposeConstBuffer(ConstBuffer* constBuffers)
{
    D3D11ConstBuffer* buffer = (D3D11ConstBuffer*)constBuffers;
    testApi->AddDisposeResource(buffer->buffer);
    delete buffer;
}

void D3D11Renderer::ApplyPipelineState(PipelineState piplineState)
{
    ApplyPixelShader(piplineState.shaders->ps);
    ApplyVertexShader(piplineState.shaders->vs);
    ApplyComputeShader(piplineState.shaders->cs);
    ApplyGeometryShader(piplineState.shaders->gs);
    if (piplineState.pipeline)
    {
        if (piplineState.pipeline->bs)
            SetBlendState(*piplineState.pipeline->bs);
        if (piplineState.pipeline->dss)
            SetDepthStencilState(*piplineState.pipeline->dss);
        if (piplineState.pipeline->rs)
            ApplyRasterizerState(*piplineState.pipeline->rs);
    }
}

void D3D11Renderer::Flush()
{}

Texture* D3D11Renderer::CreateTexture2D(int32_t width, int32_t height, int32_t levelCount, int32_t subCount,
    uint8_t isRenderTarget)
{
    return nullptr;
}

#define SET_VECTOR_NULL(vector) for(auto& elem : vector){ elem = nullptr; }

void D3D11Renderer::ClearState()
{
    depthStencilBuffer = nullptr;
    SET_VECTOR_NULL(vertexTextures);
    SET_VECTOR_NULL(pixelTextures);
    topology = PRIMITIVETYPE_UNKNOWN;
    depthStencilBuffer = nullptr;
    testApi->ClearState();
}


void D3D11Renderer::ApplyMeshBuffersBinding(const MeshBindings& bindings)
{
    ApplyVertexBufferBinding(bindings.vertexBuffer);
    ApplyIndexBufferBinding(bindings.indexBuffer, bindings.indexSize);
}


void D3D11Renderer::BeginEvent(const char* name)
{
    testApi->BeginEvent(name);
}

void D3D11Renderer::EndEvent()
{
    testApi->EndEvent();
}

void D3D11Renderer::SetMarker(const char* name)
{
    testApi->SetMarker(name);
}


void D3D11Renderer::ApplyGeometryShader(GeometryShader* geometryShader)
{
    if (geometryShader != nullptr)
    {
        D3D11GeometryShader* shader = (D3D11GeometryShader*)geometryShader;
        testApi->SetupShader(shader->testShader, GVM::EShaderType::GEOMETRY_SHADER);
    }
    else
    {
        testApi->SetupShader(nullptr, GVM::EShaderType::GEOMETRY_SHADER);
    }
}

void D3D11Renderer::ApplyComputeShader(ComputeShader* computeShader)
{
    if (computeShader != nullptr)
    {
        D3D11ComputeShader* shader = (D3D11ComputeShader*)computeShader;
        testApi->SetupShader(shader->testShader, GVM::EShaderType::COMPUTE_SHADER);
    }
    else
    {
        testApi->SetupShader(nullptr, GVM::EShaderType::COMPUTE_SHADER);
    }
}

void D3D11Renderer::Dispatch(size_t x, size_t y, size_t z)
{
    testApi->Dispatch(x, y, z);
}

void D3D11Renderer::AddDisposeGeometryShader(GeometryShader* geometryShader)
{
    D3D11GeometryShader* shader = (D3D11GeometryShader*)geometryShader;
    if (shader == nullptr)
        return;
    delete (char*)shader->data;
    testApi->AddDisposeShader(shader->testShader);
    delete shader;
}

void D3D11Renderer::AddDisposeComputeShader(ComputeShader* computeShader)
{
    D3D11PixelShader* shader = (D3D11PixelShader*)computeShader;
    if (shader == nullptr)
        return;
    delete (char*)shader->data;
    testApi->AddDisposeShader(shader->testShader);
    delete shader;
}
