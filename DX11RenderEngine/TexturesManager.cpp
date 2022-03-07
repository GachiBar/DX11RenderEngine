#include "TexturesManager.h"

using namespace Renderer;
TexturesManager::TexturesManager(Renderer::IRenderer* renderer):renderer(renderer) {
	int width, height;
	renderer->GetBackbufferSize(&width, &height);

	CreateRenderTarget(SurfaceFormat::SURFACEFORMAT_COLOR,  width, height, diffuseColor,   diffuseColorRT);
	CreateRenderTarget(SurfaceFormat::SURFACEFORMAT_COLOR,  width, height, directLights,   directLightsRT);
	CreateRenderTarget(SurfaceFormat::SURFACEFORMAT_SINGLE, width, height, blumeMask,      blumeMaskRT);
	CreateRenderTarget(SurfaceFormat::SURFACEFORMAT_SINGLE, width, height, bloomBlured,    bloomBluredRT);
	CreateRenderTarget(SurfaceFormat::SURFACEFORMAT_COLOR,  width, height, alphaSurfaces,  alphaSurfacesRT);

}

void TexturesManager::RegTexture(void* data, int width, int height, bool mipmap, size_t id) {
	auto& pTexture = textures[id];
	if (pTexture.texture != nullptr) {
		ReleaseTexture(id);
	}
	//GFX_THROW_INFO(gfx.pDevice->CreateTexture2D(&textureDesc, &sd, &pTexture.texture));
	pTexture.texture = renderer->CreateTexture2D(SurfaceFormat::SURFACEFORMAT_COLOR, width, height, 1, 0);
	renderer->SetTextureData2D(pTexture.texture, 0, 0, width, height, 0, data, width * height * 4);
	pTexture.width = width;
	pTexture.height = height;
}

void TexturesManager::RegTexture(const TextureData& tx, size_t id) {
	auto&  pTexture = textures[id];
	if (pTexture.texture != nullptr) {
		ReleaseTexture(id);
	}
	//GFX_THROW_INFO(gfx.pDevice->CreateTexture2D(&textureDesc, &sd, &pTexture.texture));
	pTexture.texture = renderer->CreateTexture2D(SurfaceFormat::SURFACEFORMAT_COLOR, tx.GetWidth(), tx.GetHeight(), 1, 0);
	renderer->SetTextureData2D(pTexture.texture, 0, 0, tx.GetWidth(), tx.GetHeight(), 0, (void*)tx.GetBufferPtr(), tx.GetWidth()*tx.GetHeight() * 4);
	pTexture.width = tx.GetWidth();
	pTexture.height = tx.GetHeight();
}

void TexturesManager::UpdateTexture(const TextureData& tx, size_t id) {
	auto& pTexture = textures[id];
	if (pTexture.texture == nullptr) {
		RegTexture(tx, id);
	}
	else {
		renderer->SetTextureData2D(pTexture.texture, 0, 0, tx.GetWidth(), tx.GetHeight(), 0, (void*)tx.GetBufferPtr(), tx.GetWidth() * tx.GetHeight() * 4);
	}
}

void TexturesManager::UpdateTexture(const ImageUpdate& updateData) {
	auto& pTexture = textures[updateData.id];
	if (pTexture.texture == nullptr) {
		pTexture.texture = renderer->CreateTexture2D(SurfaceFormat::SURFACEFORMAT_COLOR, updateData.width, updateData.height, 1, 0);
		//return;
	}
	renderer->SetTextureData2D(pTexture.texture, updateData.box.x, updateData.box.y, updateData.box.w, updateData.box.h,
		updateData.level, updateData.data, 0);
}


void TexturesManager::ReleaseTexture(size_t id) {
	auto& pTexture = textures[id];
	if (pTexture.texture != nullptr) renderer->AddDisposeTexture(pTexture.texture);
	pTexture.texture = nullptr;
	//textures.erase(id);
}

TexturesManager::~TexturesManager() {
	for (auto& [key, texture]: textures) {
		ReleaseTexture(key);
	}
	
	for (auto& texture: renderTargets) {
		renderer->AddDisposeTexture(texture);
	}


}

void TexturesManager::CreateRenderTarget(Renderer::SurfaceFormat format, size_t width, size_t height, Renderer::Texture*& texture, Renderer::RenderTargetBinding& renderTarget) {

	texture = renderer->CreateTexture2D(format, width, height, 1, true);

	renderTargets.push_back(texture);
	Viewport defVP = {
		0,0,width,height,0.0,1.0
	};


	renderTarget.viewport = defVP;
	renderTarget.texture = texture;
	renderTarget.levelCount = 1;
	renderTarget.multiSampleCount = 0;
}

TexturesManager::TextureCache TexturesManager::GetImg(size_t id) {
	return textures[id];	
}



