#ifndef OPTIXVRPATHTRACERHEAD_H
#define OPTIXVRPATHTRACERHEAD_H

#include "renderer/NodeObserver.h"

#include "world/nodes/user/oculus/OculusHeadNode.h"
#include "OptixVRPathTracerRenderer.h"

namespace Renderer::Optix
{
	struct OptixVRPathTracerHead : TypedNodeObserver<OptixVRPathTracerRenderer, World::OculusHeadNode>
	{
		OptixVRPathTracerHead(OptixVRPathTracerRenderer* renderer, World::OculusHeadNode* node);
		~OptixVRPathTracerHead() = default;

		glm::vec2 fovHalfTan;

		void UpdateFromNode() override;
		void UpdateFromVisual(RenderTarget* target) override;
	};
}
#endif // OPTIXVRPATHTRACERHEAD_H
