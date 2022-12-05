#pragma once
#include "CoreRenderSystem/RenderPasses/BaseRenderPass.h"
#include "LightPassProvider.h"
#include "ResourceManagers/ModelsManager.h"
#include "ResourceManagers/TexturesManager.h"
#include "LightPassConstBuffer.h"
#include "CoreRenderSystem/RenderPasses/QuadRenderPass.h"
#include "RendererPasses/GachiBasePass.h"
#include "Utils/OpaqueModelDrawData.h"

class LightRenderPass : public GachiBasePass{

    QuadRenderPass<0> quad;
    struct DrawCall {
        DrawCall(
            ModelsManager::SavedModel model,
            const LightDrawData&
            );

        ModelsManager::SavedModel model;
        LightDrawData data;
    };	
	
    PipelineFactoryFlags ParseFlags(const LightDrawData& flags);
    
public:
    explicit LightRenderPass(BaseRenderSystem& in);
    
    void Draw(const LightDrawData& drawData);
    
    void Init(const char* dirr) override;
    void PreRender() override;


    void Resize() override;
    void Render();
    
    void PostRender() override;
    ~LightRenderPass() override;
    void SetupSettings(const RenderSettings& Settings) override;

private:
    LightConstBuffer dataBuffer;

    Renderer::ConstBuffer* pDataCB;

    std::vector<DrawCall> drawCalls;
	Renderer::Viewport vp;
};
