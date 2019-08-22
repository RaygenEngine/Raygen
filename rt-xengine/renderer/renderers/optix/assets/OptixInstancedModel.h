#ifndef OPTIXINSTANCEDMODEL_H
#define OPTIXINSTANCEDMODEL_H

#include "OptixModel.h"

namespace Renderer::Optix
{

	class OptixInstancedModel : public OptixModel
	{
		// instancing transforms
		std::vector<optix::Transform> m_transforms;

	public:
		OptixInstancedModel(OptixRendererBase* renderer);
		~OptixInstancedModel() = default;

		bool Load(Assets::XModel* data, uint32 closestHitRayType, Assets::StringFile* closestHitProgramSource,
			std::string closestHitProgramName, uint32 anyHitRayType, Assets::StringFile* anyHitProgramSource,
			std::string anyHitProgramName, World::TriangleModelInstancedGeometryNode* nodeInstancer);

		const std::vector<optix::Transform>& GetInstancingTransforms() const { return m_transforms; }

		void ToString(std::ostream& os) const override { os << "asset-type: OptixInstancedModel, name: " << m_associatedDescription; }
	};

}

#endif // OPTIXINSTANCEDMODEL_H
