#ifndef OPTIXCUBEMAP_H
#define OPTIXCUBEMAP_H

#include "renderer/renderers/optix/OptixAsset.h"

namespace Renderer::Optix
{

	class OptixCubeMap : public OptixAsset
	{

		optix::TextureSampler m_handle;
	
	public:
		OptixCubeMap(OptixRendererBase* renderer);
		~OptixCubeMap() = default;

		bool Load(Assets::CubeMap* data);

		optix::TextureSampler GetOptixHandle() const { return m_handle; }

		void ToString(std::ostream& os) const override { os << "asset-type: OptixCubeMap, name: " << m_associatedDescription; }
	};

}

#endif // OPTIXCUBEMAP_H
