#ifndef OPTIXVRPATHTRACERGEOMETRY_H
#define OPTIXVRPATHTRACERGEOMETRY_H

#include "renderer/NodeObserver.h"
#include "world/nodes/geometry/TriangleModelGeometryNode.h"
#include "renderer/renderers/optix/assets/OptixModel.h"
#include "OptixVRPathTracerRenderer.h"

namespace Renderer::Optix
{
	struct OptixVRPathTracerGeometry : TypedNodeObserver<OptixVRPathTracerRenderer, World::TriangleModelGeometryNode>
	{

		std::shared_ptr<OptixModel> optixModel;
		optix::Transform transform;

		OptixVRPathTracerGeometry(OptixVRPathTracerRenderer* renderer, World::TriangleModelGeometryNode* node);
		~OptixVRPathTracerGeometry() = default;

		void UpdateFromNode() override;
	};
}

#endif // OPTIXVRPATHTRACERGEOMETRY_H
