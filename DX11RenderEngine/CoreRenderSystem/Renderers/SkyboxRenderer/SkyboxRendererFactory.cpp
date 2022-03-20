#include "SkyboxRendererFactory.h"


using namespace Renderer;




SkyboxRendererFactory::SkyboxRendererFactory(Renderer::IStateProvider* provider, void* shaderData, size_t dataSize)
	:PipelineFactory(provider, (const Renderer::ShaderDefines*)SkyboxRendererDefines, std::size(SkyboxRendererDefines), shaderData, dataSize, 
#ifdef _DEBUG 
		D3DCOMPILE_DEBUG
#else
0
#endif
	)
{

}
