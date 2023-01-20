#pragma once
#include "CoreRenderSystem/RenderPasses/BaseRenderPass.h"
#include "OpaquePassProvider.h"
#include "ResourceManagers/ModelsManager.h"
#include "ResourceManagers/TexturesManager.h"
#include "OpaquePassConstBuffer.h"
#include "RendererPasses/GachiBasePass.h"
#include "Utils/OpaqueModelDrawData.h"

class OpaqueRenderPass : public GachiBasePass{

    struct DrawCall {
        DrawCall(ModelsManager::SavedModel model,
            
        TexturesManager::TextureCache diffuse,
        TexturesManager::Float3TextureCache normal,
        TexturesManager::FloatTextureCache roughness,
        TexturesManager::FloatTextureCache metallic,
        
            const OpaqueModelDrawData&);

        ModelsManager::SavedModel model;
        TexturesManager::TextureCache diffuse;
        TexturesManager::Float3TextureCache normal;
        TexturesManager::FloatTextureCache roughness;
        TexturesManager::FloatTextureCache metallic;
        OpaqueModelDrawData data;
    };	
	
    PipelineFactoryFlags ParseFlags(const OpaqueModelDrawData& flags);
    
public:
    explicit OpaqueRenderPass(BaseRenderSystem& in);
    
    void Draw(const OpaqueModelDrawData& drawData);
    
    void Init(const char* dirr) override;
    void PreRender() override;


    void Resize() override;
    void Render();
    
    void PostRender() override;
    ~OpaqueRenderPass() override;
    void SetupSettings(const RenderSettings& Settings) override;
    std::vector<std::vector<uint>> idData;

private:
    OpaqueCosntBuffer dataBuffer;

    Renderer::ConstBuffer* pDataCB;

    std::vector<DrawCall> drawCalls;
	Renderer::Viewport vp;
};
