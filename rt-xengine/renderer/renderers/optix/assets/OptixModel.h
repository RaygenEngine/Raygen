#ifndef OPTIXMODEL_H
#define OPTIXMODEL_H

#include "renderer/renderers/optix/OptixAsset.h"


namespace Renderer::Optix
{
	// triangle model
	class OptixModel : public OptixAsset
	{
	protected:
		optix::GeometryGroup m_handle;

		std::vector<std::shared_ptr<OptixMesh>> m_meshes;

	public:
		OptixModel(OptixRendererBase* renderer);
		virtual ~OptixModel() = default;

		bool Load(Assets::XModel* data, uint32 closestHitRayType, Assets::StringFile* closestHitProgramSource,
			std::string closestHitProgramName, uint32 anyHitRayType, Assets::StringFile* anyHitProgramSource,
			std::string anyHitProgramName);

		optix::GeometryGroup GetOptixHandle() const { return m_handle; }

		void ToString(std::ostream& os) const override { os << "asset-type: OptixModel, name: " << m_associatedDescription; }
	};

}

#endif // OPTIXMODEL_H
