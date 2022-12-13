#include "D3D11Renderer.h"
#include "BaseRenderSystem.h"
#include "GDIPlusManager.h"

using namespace Renderer;


BaseRenderSystem::BaseRenderSystem(const BaseRenderSystemInitStruct& init):
pRenderer(init.pRenderer), texturesManger(init.texturesManger), modelsManager(init.modelsManager)
{

	pLocalConstants = pRenderer->CreateConstBuffer(sizeof(viewConstants));
	viewConstants.time = 0;
	//pRenderer->VerifyConstBuffer(pLocalConstants, coreConstants.slot);
	gpmt = new GDIPlusManager();
}


bool BaseRenderSystem::Present()
{
	pRenderer->SetConstBuffer(pLocalConstants, &viewConstants);
	pRenderer->VerifyConstBuffer(pLocalConstants, coreConstants.slot);
	return true;
}

void BaseRenderSystem::SetRenderData(const RenderData& data){
	viewConstants.dt = data.time - viewConstants.time; 
	viewConstants.time = data.time;
	viewConstants.viewPosition = data.viewPosition;
	
	viewConstants.pastMatrices = viewConstants.currentMatrices;
	
	auto& currentMatrices = viewConstants.currentMatrices;
	
	currentMatrices.view = data.view.Transpose();
	currentMatrices.inverseView = data.view.Invert().Transpose();
	

	currentMatrices.projection = data.projection.Transpose();
	currentMatrices.inverseProjection = data.projection.Invert().Transpose();

	matrix viewProjection = data.view * data.projection;
	currentMatrices.viewProjection = viewProjection;//.Transpose();
	currentMatrices.viewProjectionInverse = viewProjection.Invert().Transpose();
	
	
	currentMatrices.reprojectionMatrix = viewProjection.InvertHighPrecision() * viewConstants.pastMatrices.viewProjection;

	
	
	pRenderer->SetConstBuffer(pLocalConstants, &viewConstants);
	
}

BaseRenderSystem::~BaseRenderSystem()
{
	delete gpmt;
	delete pRenderer;
	delete texturesManger;
	delete modelsManager;
}

