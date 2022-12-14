#pragma once
#include "TransformUtils.h"
#include "SimpleMath.h"
#include "CoreShaderInclude.h"
#include "IRenderer.h"
#include "ResourceManagers/IModelsManager.h"
#include "ResourceManagers/ITexturesManager.h"


class GDIPlusManager;


class BaseRenderSystem {

	GDIPlusManager* gpmt;
	
protected:
	
	struct BaseRenderSystemInitStruct {
		Renderer::IRenderer* pRenderer;
		ITexturesManager* texturesManger;
		IModelsManager<>* modelsManager;
		
	};
	
	
	BaseRenderSystem(const BaseRenderSystemInitStruct&);

private:
	
	Renderer::ConstBuffer* pLocalConstants;

public:
	CoreConstants viewConstants;
	Renderer::IRenderer* pRenderer;
	ITexturesManager* texturesManger;
	IModelsManager<>* modelsManager;
	
	const CoreConstants& GetViewConstants() {return viewConstants;};
	
	void Setup();
	
	void SetRenderData(const RenderData& data);
	
	virtual ~BaseRenderSystem();
};