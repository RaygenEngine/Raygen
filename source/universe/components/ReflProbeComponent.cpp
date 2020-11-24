#include "ReflProbeComponent.h"

#include "rendering/scene/SceneReflprobe.h"

DECLARE_DIRTY_FUNC(CReflprobe)(BasicComponent& bc)
{
	auto build = notifyBuild;
	notifyBuild = false;

	return [=, position = bc.world().position](SceneReflprobe& rp) {
		rp.ubo.position = glm::vec4(position, 1.f);

		if (FullDirty) {

			rp.ubo.radius = radius;

			rp.ptSamples = ptSamples;
			rp.ptBounces = ptBounces;

			rp.irrResolution = irrResolution;

			rp.ubo.lodCount = prefLodCount;
			rp.prefSamples = prefSamples;

			rp.ubo.irradianceFactor = applyIrradiance;

			if (build) {
				rp.Allocate();
			}
		}
	};
}
