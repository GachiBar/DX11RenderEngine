#pragma once
#include "PipelineFactory.h"
#include "UPRendererUtils.h"
enum UPDefines  : uint64_t {
    UPZERO = 0,
    UPRED = 1,
    UPALPHA = 2,
    UPWATER = 4,
};


class UPRendererFactory : public Renderer::PipelineFactory  {
public:
    UPRendererFactory(Renderer::IRenderer* renderer, Renderer::IStateProvider* provider, void* shaderData, size_t dataSize);
    static const Renderer::ShaderDefines UPRendererDefines[];
};

