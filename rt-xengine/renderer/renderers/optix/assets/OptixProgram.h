#ifndef OPTIXPROGRAM_H
#define OPTIXPROGRAM_H

#include "renderer/renderers/optix/OptixAsset.h"

namespace Renderer::Optix
{

	class OptixProgram : public OptixAsset
	{

		optix::Program m_handle;

		std::string m_name;

	public:
		OptixProgram(OptixRendererBase* renderer);
		~OptixProgram() = default;

		bool Load(Assets::StringFile* sourceFile, const std::string& programName);

		optix::Program GetOptixHandle() const { return m_handle; };

		void ToString(std::ostream& os) const override { os << "asset-type: OptixProgram, src name: " << m_associatedDescription << ", program name: " << m_name; }
	};

}

#endif // OPTIXPROGRAM_H
