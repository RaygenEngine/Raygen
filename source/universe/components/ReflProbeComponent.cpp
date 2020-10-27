#include "ReflProbeComponent.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/scene/SceneReflprobe.h"

DECLARE_DIRTY_FUNC(CReflprobe)(BasicComponent& bc)
{
	auto build = shouldBuild;
	shouldBuild = false;

	return [=, position = bc.world().position](SceneReflprobe& rp) {
		rp.position = glm::vec4(position, 1.f);

		if (FullDirty) {

			rp.innerRadius = innerRadius;
			rp.outerRadius = outerRadius;

			rp.ptSamples = ptSamples;
			rp.ptBounces = ptBounces;

			rp.ubo.lodCount = prefLodCount;

			rp.shouldBuild.Assign(build);

			rp.ShouldResize();
		}
	};
}
