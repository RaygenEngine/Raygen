#include "ReflProbeComponent.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/scene/SceneReflprobe.h"

DECLARE_DIRTY_FUNC(CReflprobe)(BasicComponent& bc)
{
	return [=, position = bc.world().position](SceneReflprobe& rp) {
		rp.position = position;

		if (FullDirty) {
			rp.envmap = vl::GpuAssetManager->GetGpuHandle(environmentMap);
		}
	};
}
