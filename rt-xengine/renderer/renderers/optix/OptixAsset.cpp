#include "pch.h"

#include "OptixAsset.h"

namespace Renderer::Optix
{
	OptixAsset::OptixAsset(OptixRendererBase* renderer)
		: GPUAsset(renderer),
		m_renderer(renderer)
	{
	}
}
