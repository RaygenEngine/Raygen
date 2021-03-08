#include "StaticMeshComponent.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/SceneGeometry.h"

DECLARE_DIRTY_FUNC(CStaticMesh)(BasicComponent& bc)
{
	return [=, transform = bc.world().transform](SceneGeometry& geom) {
		geom.transform = transform;
		if constexpr (FullDirty) {
			geom.mesh = vl::GpuAssetManager->GetGpuHandle(mesh);
		}
	};
}
