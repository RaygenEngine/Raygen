#include "pch.h"
#include "ReflectionProbeComponent.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/scene/SceneReflectionProbe.h"

DECLARE_DIRTY_FUNC(CReflectionProbe)(BasicComponent& bc)
{
	return [=](SceneReflectionProbe& rp) {
		if (FullDirty) {
			rp.envmap = vl::GpuAssetManager->GetGpuHandle(environmentMap);
		}
	};
}
