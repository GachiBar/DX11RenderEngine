#include "UPRenderer.h"


using namespace Renderer;


UPRenderer::UPRenderer() :
	renderer(IRenderer::renderer), staticMeshes(20000, 20000), dynamicMeshes(2000, 2000) {}

void UPRenderer::Init(void* shaderData, size_t dataSize) {
	if (provider != nullptr) {
		//delete provider;
		delete factory;
		int32_t width, height;
		renderer->GetBackbufferSize(&width, &height);
		provider = new UPRendererProvider(width, height);
		factory = new UPRendererFactory(renderer, provider, shaderData, dataSize);
		return;
	}

	int32_t width, height;
	renderer->GetBackbufferSize(&width, &height);
	provider = new UPRendererProvider(width, height);
	factory = new UPRendererFactory(renderer, provider, shaderData, dataSize);

	pDataCB = renderer->CreateConstBuffer(sizeof(dataBuffer));


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

void UPRenderer::Init(LPCWSTR dirr) {
	wrl::ComPtr<ID3DBlob> buff;
	D3DReadFileToBlob((std::wstring(dirr) + L"\\BSPShader.hlsl").c_str(), &buff);
	auto data = buff->GetBufferPointer();
	auto size = buff->GetBufferSize();
	Init(data, size);
}

MeshHashData UPRenderer::Register(UPModelData model, bool dynamic) {
	if (dynamic) {
		return dynamicMeshes.AddMesh(model);
	}
	else {
		return staticMeshes.AddMesh(model);
	}
}

void UPRenderer::Draw(MeshHashData model, TexturesManager::TextureCache texture, UPDrawData data) {
	drawCalls.emplace_back(model, texture, data);
}
void UPRenderer::Draw(MeshHashData model, TexturesManager::TextureCache texture, TexturesManager::TextureCache lightMap, UPDrawData data) {
	drawCalls.emplace_back(model, texture, lightMap, data);
}

void UPRenderer::DrawSet(MeshHashData model, UPModelData newModel, TexturesManager::TextureCache texture, UPDrawData data) {

	if (data.dynamic) {
		dynamicMeshes.UpdateMesh(model, newModel);
	}
	else {
		printf("Static Mesh Updated. Fix this !!!!!!!!!!!!\n");
		staticMeshes.UpdateMesh(model, newModel);
	}

	drawCalls.emplace_back(model, texture, data);
}

void UPRenderer::Render(GraphicsBase& gfx) {
	
	RenderTargetBinding* targets[3] = {
		 &gfx.texturesManger.diffuseColorRT,
		 &gfx.texturesManger.lightColorRT,
		 &gfx.texturesManger.alphaSurfacesRT,
	};

	renderer->SetRenderTargets(targets, 3, gfx.texturesManger.depthBuffer, Viewport());

	staticMeshes.UpdateBuffers();
	dynamicMeshes.UpdateBuffers(true);



	int32_t width, height;
	renderer->GetBackbufferSize(&width, &height);
	size_t lastFlags = -1;

	renderer->VerifyConstBuffer(pDataCB, upCosntBuffer.slot);

	renderer->VerifyPixelSampler(0, Samplers::anisotropic16);
	renderer->VerifyPixelSampler(1, Samplers::anisotropic16);



	for (size_t i = 0; i < drawCalls.size(); i++) {
		if (drawCalls[i].data.flags != lastFlags) {
			renderer->ApplyPipelineState(factory->GetState(drawCalls[i].data.flags));
		}
		lastFlags = drawCalls[i].data.flags;



		auto  pTexture = drawCalls[i].texture.texture;
		renderer->VerifyPixelTexture(0, pTexture);
		auto  pLightMap = drawCalls[i].lightMap.texture;
		renderer->VerifyPixelTexture(1, pLightMap);
	

		dataBuffer.world = drawCalls[i].data.position.GetTransform();
		dataBuffer.texOffset = drawCalls[i].data.texOffset;
		dataBuffer.color = drawCalls[i].data.light;
		renderer->SetConstBuffer(pDataCB, &dataBuffer);


		if (drawCalls[i].data.dynamicLight)
		{
			gfx.texturesManger.UpdateTexture(*drawCalls[i].data.lightUpdate);
			delete[] drawCalls[i].data.lightUpdate->data;
			delete drawCalls[i].data.lightUpdate;
		}

		if (drawCalls[i].data.dynamic) {
			renderer->ApplyMeshBuffersBinding(dynamicMeshes.mesh);
			renderer->DrawIndexedPrimitives(
				drawCalls[i].model.pt, 0, 0, 0, drawCalls[i].model.indexOffset,
				drawCalls[i].model.numElem);
		}
		else {
			renderer->ApplyMeshBuffersBinding(staticMeshes.mesh);
			renderer->DrawIndexedPrimitives(
				drawCalls[i].model.pt, 0, 0, 0, drawCalls[i].model.indexOffset,
				drawCalls[i].model.numElem);
		}

	}
}

void UPRenderer::Flush() {
	staticMeshes.Flush();
	dynamicMeshes.Flush();
}

void UPRenderer::Clear(GraphicsBase& gfx) {
	drawCalls.clear();


	RenderTargetBinding* targets[3] = {
		 &gfx.texturesManger.diffuseColorRT,
		 &gfx.texturesManger.lightColorRT,
		 &gfx.texturesManger.alphaSurfacesRT,
	};

	renderer->SetRenderTargets(targets, 3, gfx.texturesManger.depthBuffer, Viewport());
	renderer->Clear(ClearOptions::CLEAROPTIONS_TARGET, { 0, 0, 0, 0 }, 0, 0);
}

void UPRenderer::Destroy() {

	renderer->AddDisposeConstBuffer(pDataCB);
	delete factory;
}


UPRenderer::~UPRenderer() { Destroy(); }

UPRenderer::DrawCall::DrawCall(MeshHashData model, TexturesManager::TextureCache texture, UPDrawData data) :
	model(model), texture(texture), data(data) {}

UPRenderer::DrawCall::DrawCall(MeshHashData model, TexturesManager::TextureCache texture, TexturesManager::TextureCache lightMap, UPDrawData data) :
	DrawCall(model, texture, data) {
	this->lightMap = lightMap;
}
