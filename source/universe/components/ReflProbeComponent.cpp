#include "ReflProbeComponent.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/scene/SceneReflprobe.h"

DECLARE_DIRTY_FUNC(CReflprobe)(BasicComponent& bc)
{
	auto build = shouldBuild;
	shouldBuild = false;

	return [=, position = bc.world().position](SceneReflprobe& rp) {
		rp.ubo.position = glm::vec4(position, 1.f);

		if (FullDirty) {
			// rp.envmap = vl::GpuAssetManager->GetGpuHandle(environmentMap);
			rp.ubo.innerRadius = innerRadius;
			rp.ubo.outerRadius = outerRadius;

			rp.shouldBuild.Assign(build);

			rp.ShouldResize(resolution);
		}
	};
}
