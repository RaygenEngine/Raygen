#ifndef OPTIXASSET_H
#define OPTIXASSET_H

#include "OptixRendererBase.h"
#include "renderer/GPUAsset.h"

namespace Renderer::Optix
{
	
	class OptixAsset : public GPUAsset
	{
	protected:
		OptixRendererBase* m_renderer;

	public:
		OptixAsset(OptixRendererBase* renderer);
		virtual ~OptixAsset() = default;

		inline optix::Context GetOptixContext() const { return m_renderer->GetOptixContext(); }
		inline OptixRendererBase* GetRenderer() const { return m_renderer; }
	};

}

#endif // OPTIXASSET_H
