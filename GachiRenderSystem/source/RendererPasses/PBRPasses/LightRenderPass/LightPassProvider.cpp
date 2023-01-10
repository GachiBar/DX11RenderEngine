#include "LightPassProvider.h"

#include <d3d11.h>
#include <array>

#include "RenderFlags.h"
#include "ResourceManagers/States/BlendStates.h"
#include "ResourceManagers/States/DSStates.h"
#include "ResourceManagers/States/RasterizerStates.h"
#include "Utils/DrawData.h"
void LightPassProvider::PatchPipelineState(Renderer::Pipeline* refToPS, uint32_t pipelineFlags)
{
    LightFlags flags;
    flags.flags = pipelineFlags;
    refToPS->bs = &BlendStates::Light;

    refToPS->dss = &DepthStencilStates::NoDSS;

    refToPS->rs = &RasterizerStates::CClockWise;

   //if (flags.isWireframe)
   //{
   //    refToPS->rs = &RasterizerStates::Wireframe;
   //}
}

const D3D11_INPUT_ELEMENT_DESC SQInputElements[] =
    {
    {"Position",  0, DXGI_FORMAT_R32G32_FLOAT,  0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
};
const D3D11_INPUT_ELEMENT_DESC DefaultInputElements[] =
    {
    {"Position",  0, DXGI_FORMAT_R32G32_FLOAT,  0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
};



Renderer::InputLayoutDescription LightPassProvider::GetInputLayoutDescription(uint32_t definesFlags)
{
    if (definesFlags & (uint)PixelFlagsLighting::SCREEN_QUAD)
        return Renderer::InputLayoutDescription{ (void*)SQInputElements, std::size(DefaultInputElements) };
    else
        return Renderer::InputLayoutDescription{ (void*)DefaultInputElements, std::size(DefaultInputElements) };
}


const Renderer::ShaderDefines ModelRendererDefines[] = {
    {"AmbientLight", "1"},
    {"DirectionalLight", "1"},
    {"AmbientLight", "1"},
    {"SpotLight", "1"},
    {"SCREEN_QUAD", "1"},
};


Renderer::PipelineFactoryDescription LightPassProvider::GetFactoryDescription()
{
    return {
        ModelRendererDefines,
        std::size(ModelRendererDefines)
    };
}
const char* LightPassProvider::GetShaderName()
{
    return "OpaqueShader";
}
LightPassProvider::~LightPassProvider()
{
    
}
