#pragma once
#include "IRenderer.h"
//#include "RenderFlags.h"
#include <map>
#include <vector>

enum UsedShaders: uint16_t {
    UseGeometryShader = 1,
    UseComputeShader = 2,
};

struct PipelineFactoryFlags {
    union {
        size_t flags = 0;
        struct {
            uint32_t definesFlags;
            uint32_t pipelineFlags;
        };
    };
};

class PipelineFactory {

    Renderer::IRenderer* renderDevice;
    Renderer::IStateProvider* provider;

    void* shaderData;
    size_t dataSize;
    uint16_t compileFlags;

    const Renderer::ShaderDefines* defines;
    size_t defineCount;

    uint32_t useShaders;

    std::map<uint32_t, Renderer::Pipeline*> dictinaryPipeline;
    std::map<uint32_t, Renderer::PipelineShaders*> dictinaryShaders;
    std::map<std::pair<uint32_t, const char*>, Renderer::PipelineShaders*> computeDictinary;



    PipelineFactory(Renderer::IStateProvider* provider, const Renderer::ShaderDefines* defines, size_t defineCount, uint16_t compileFlags);
public:
    PipelineFactory(
        Renderer::IRenderer* renderDevice,
        Renderer::IStateProvider* provider,
        void* shaderData,
        size_t dataSize);

    std::vector<Renderer::ShaderDefines> GetDefines(uint32_t definesFlags);

    Renderer::PipelineState GetState(PipelineFactoryFlags definesFlags);
    Renderer::PipelineState GetComputeState(uint32_t definesFlags, const char* name);


    ~PipelineFactory();
};
