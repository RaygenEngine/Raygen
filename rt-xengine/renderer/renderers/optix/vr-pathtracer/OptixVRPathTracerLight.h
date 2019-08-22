#ifndef OPTIXVRPATHTRACERLIGHT_H
#define OPTIXVRPATHTRACERLIGHT_H

#include "renderer/NodeObserver.h"
#include "world/nodes/light/LightNode.h"
#include "core/data/BasicLight.h"
#include "OptixVRPathTracerRenderer.h"

namespace Renderer::Optix
{
	struct OptixVRPathTracerLight : TypedNodeObserver<OptixVRPathTracerRenderer, World::LightNode>
	{
		Core::BasicLight light;

		OptixVRPathTracerLight(OptixVRPathTracerRenderer* renderer, World::LightNode* node);
		~OptixVRPathTracerLight() = default;

		void UpdateFromNode() override;
	};
}

#endif // OPTIXVRPATHTRACERLIGHT_H
