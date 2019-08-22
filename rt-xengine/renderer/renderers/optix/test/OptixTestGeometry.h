#ifndef OPTIXTESTGEOMETRY_H
#define OPTIXTESTGEOMETRY_H

#include "renderer/NodeObserver.h"
#include "world/nodes/geometry/TriangleModelGeometryNode.h"
#include "renderer/renderers/optix/assets/OptixModel.h"
#include "OptixTestRenderer.h"

namespace Renderer::Optix
{
	struct OptixTestGeometry : TypedNodeObserver<OptixTestRenderer, World::TriangleModelGeometryNode>
	{

		std::shared_ptr<OptixModel> optixModel;
		optix::Transform transform;

		OptixTestGeometry(OptixTestRenderer* renderer, World::TriangleModelGeometryNode* node);
		~OptixTestGeometry() = default;

		void UpdateFromNode() override;
	};
}

#endif // OPTIXTESTGEOMETRY_H
