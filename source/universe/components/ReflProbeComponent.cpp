#include "ReflProbeComponent.h"

#include "rendering/scene/SceneReflprobe.h"

DECLARE_DIRTY_FUNC(CReflprobe)(BasicComponent& bc)
{
	auto build = notifyBuild;
	notifyBuild = false;

	return [=, position = bc.world().position](SceneReflprobe& rp) {
		rp.ubo.position = glm::vec4(position, 1.f);

		if (FullDirty) {

			rp.ubo.innerRadius = innerRadius;
			rp.ubo.outerRadius = outerRadius;

			rp.ptSamples = ptSamples;
			rp.ptBounces = ptBounces;

			rp.irrResolution = irrResolution;

			rp.ubo.lodCount = prefLodCount;
			rp.prefSamples = prefSamples;

			if (build) {
				rp.Allocate();
			}
		}
	};
}
