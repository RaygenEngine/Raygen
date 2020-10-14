#include "StaticMeshComponent.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/scene/SceneGeometry.h"

DECLARE_DIRTY_FUNC(CStaticMesh)(BasicComponent& bc)
{
	return [=](SceneGeometry& geom) {
		geom.transform = bc.world().transform;
		if constexpr (FullDirty) {
			geom.mesh = vl::GpuAssetManager->GetGpuHandle(mesh);
		}
	};
}
