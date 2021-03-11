#include "StaticMeshComponent.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/SceneGeometry.h"

DECLARE_DIRTY_FUNC(CStaticMesh)(BasicComponent& bc)
{
	return [=, transform = bc.world().transform()](SceneGeometry& geom) {
		XMStoreFloat4x4A(&geom.transform, transform);

		if constexpr (FullDirty) {
			geom.mesh = GpuAssetManager->GetGpuHandle(mesh);
		}
	};
}
