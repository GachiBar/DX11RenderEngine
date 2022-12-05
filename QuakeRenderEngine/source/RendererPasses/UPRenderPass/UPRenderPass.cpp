#include "pch.h"
#include "UPRenderPass.h"

#include "ResourceManagers/States/Samplers.h"


using namespace Renderer;


UPRenderPass::UPRenderPass(BaseRenderSystem& renderSystem) :
BaseRenderPass({"BSPShader.hlsl", renderSystem}),
staticMeshes(renderSystem.pRenderer, 20000, 20000), dynamicMeshes(renderSystem.pRenderer, 2000, 2000)
{
	pDataCB = renderDevice->CreateConstBuffer(sizeof(dataBuffer));


	sampler.filter = TextureFilter::TEXTUREFILTER_POINT;
	sampler.addressU = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;
	sampler.addressV = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;
	sampler.addressW = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;
	sampler.mipMapLevelOfDetailBias = 0;

	lightSampler.filter = TextureFilter::TEXTUREFILTER_ANISOTROPIC;
	lightSampler.addressU = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;
	lightSampler.addressV = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;
	lightSampler.addressW = TextureAddressMode::TEXTUREADDRESSMODE_WRAP;
	lightSampler.mipMapLevelOfDetailBias = 0;
	lightSampler.maxAnisotropy = 16;
}

void UPRenderPass::Init(const char* dirr) {
    BaseRenderPass::Init(dirr, new UPRenderPassProvider());
}

MeshHashData UPRenderPass::Register(UPModelMesh model, bool dynamic) {
	if (dynamic) {
		return dynamicMeshes.AddMesh(model);
	}
	else {
		return staticMeshes.AddMesh(model);
	}
}

void UPRenderPass::Draw(MeshHashData model, TexturesManager::TextureCache texture, UPDrawData data) {
	drawCalls.emplace_back(model, texture, data);
}
void UPRenderPass::Draw(MeshHashData model, TexturesManager::TextureCache texture, TexturesManager::TextureCache lightMap, UPDrawData data) {
	drawCalls.emplace_back(model, texture, lightMap, data);
}

void UPRenderPass::DrawSet(MeshHashData model, UPModelMesh newModel, TexturesManager::TextureCache texture, UPDrawData data) {

	if (data.dynamic) {
		dynamicMeshes.UpdateMesh(model, newModel);
	}
	else {
		printf("Static Mesh Updated. Fix this !!!!!!!!!!!!\n");
		staticMeshes.UpdateMesh(model, newModel);
	}

	drawCalls.emplace_back(model, texture, data);
}

void UPRenderPass::Render() {
	
	RenderTargetBinding* targets[] = {
		 baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("diffuseColor")),
		 baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("lightColor")),
		 baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("alphaSurfaces")),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("velocityField")),
        baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("normalsField")),
	};

	renderDevice->SetRenderTargets(targets, std::size(targets),
		baseRendererParams.renderSystem.texturesManger->depthBuffer);

	staticMeshes.UpdateBuffers();
	dynamicMeshes.UpdateBuffers(true);



	uint32_t width, height;
	renderDevice->GetBackbufferSize(width, height);
	size_t lastFlags = -1;

	renderDevice->VerifyConstBuffer(pDataCB, upCosntBuffer.slot);

	renderDevice->VerifyPixelSampler(0, Samplers::pointClamp);
	renderDevice->VerifyPixelSampler(1, Samplers::linearClamp);



	for (size_t i = 0; i < drawCalls.size(); i++) {
		if (drawCalls[i].data.flags != lastFlags) {
			renderDevice->ApplyPipelineState(factory->GetState(drawCalls[i].data.flags));
		}
		lastFlags = drawCalls[i].data.flags;



		auto  pTexture = drawCalls[i].texture.texture;
		renderDevice->VerifyPixelTexture(0, pTexture);
		auto  pLightMap = drawCalls[i].lightMap.texture;
		renderDevice->VerifyPixelTexture(1, pLightMap);
	

		dataBuffer.world = drawCalls[i].data.position.GetTransform();
		dataBuffer.texOffset = drawCalls[i].data.texOffset;
		dataBuffer.color = drawCalls[i].data.light;
		renderDevice->SetConstBuffer(pDataCB, &dataBuffer);


		if (drawCalls[i].data.dynamicLight)
		{
			baseRendererParams.renderSystem.texturesManger->UpdateTexture(*drawCalls[i].data.lightUpdate);
		}

		if (drawCalls[i].data.dynamic) {
			renderDevice->ApplyMeshBuffersBinding(dynamicMeshes.mesh);
			renderDevice->DrawIndexedPrimitives(
				(Renderer::PrimitiveType)drawCalls[i].model.pt, 0, 0, 0, drawCalls[i].model.indexOffset,
				drawCalls[i].model.numElem);
		}
		else {
			renderDevice->ApplyMeshBuffersBinding(staticMeshes.mesh);
			renderDevice->DrawIndexedPrimitives(
				(Renderer::PrimitiveType)drawCalls[i].model.pt, 0, 0, 0, drawCalls[i].model.indexOffset,
				drawCalls[i].model.numElem);
		}

	}
}

void UPRenderPass::Flush() {
	staticMeshes.Flush();
	dynamicMeshes.Flush();
}

void UPRenderPass::PostRender()
{
	for (auto& drawCall: drawCalls)
	{
		if (drawCall.data.dynamicLight)
		{
			delete[] drawCall.data.lightUpdate->data;
			delete drawCall.data.lightUpdate;
		}
	}

	drawCalls.clear();

}

void UPRenderPass::Destroy() {

	renderDevice->AddDisposeConstBuffer(pDataCB);
	delete factory;
}
void UPRenderPass::PreRender()
{
	
	RenderTargetBinding* targets[] = {
		baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("diffuseColor")),
		baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("lightColor")),
		baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("alphaSurfaces")),
		baseRendererParams.renderSystem.texturesManger->GetRenderTarget(SID("normalsField"))
	};

	renderDevice->SetRenderTargets(targets, std::size(targets), baseRendererParams.renderSystem.texturesManger->depthBuffer);
	renderDevice->Clear(ClearOptions::CLEAROPTIONS_TARGET, { 0, 0, 0, 0 }, 0, 0);
}


UPRenderPass::~UPRenderPass() { Destroy(); }

UPRenderPass::DrawCall::DrawCall(MeshHashData model, TexturesManager::TextureCache texture, UPDrawData data) :
	model(model), texture(texture), data(data) {}

UPRenderPass::DrawCall::DrawCall(MeshHashData model, TexturesManager::TextureCache texture, TexturesManager::TextureCache lightMap, UPDrawData data) :
	DrawCall(model, texture, data) {
	this->lightMap = lightMap;
}
