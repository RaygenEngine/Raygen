#ifndef OPTIXTEXTURE_H
#define OPTIXTEXTURE_H

#include "renderer/renderers/optix/OptixAsset.h"

namespace Renderer::Optix
{

	class OptixTexture : public OptixAsset
	{

		optix::TextureSampler m_handle;
	
	public:
		OptixTexture(OptixRendererBase* renderer);
		~OptixTexture() = default;

		bool Load(Assets::Texture* data);

		optix::TextureSampler GetOptixHandle() const { return m_handle; }

		void ToString(std::ostream& os) const override { os << "asset-type: OptixTexture, name: " << m_associatedDescription; }
	};

}

#endif // OPTIXTEXTURE_H
