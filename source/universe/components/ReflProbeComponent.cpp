#include "ReflProbeComponent.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/scene/SceneReflProbe.h"

DECLARE_DIRTY_FUNC(CReflProbe)(BasicComponent& bc)
{
	return [=, position = bc.world().position](SceneReflProbe& rp) {
		rp.position = position;

		if (FullDirty) {
			rp.envmap = vl::GpuAssetManager->GetGpuHandle(environmentMap);
		}
	};
}
