#include "pch.h"
#include "ReflProbeComponent.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/scene/SceneReflProbe.h"

DECLARE_DIRTY_FUNC(CReflProbe)(BasicComponent& bc)
{
	return [=](SceneReflProbe& rp) {
		if (FullDirty) {
			rp.envmap = vl::GpuAssetManager->GetGpuHandle(environmentMap);
		}
	};
}
