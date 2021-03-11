#include "SkinnedMeshComponent.h"

#include "assets/pods/SkinnedMesh.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/SceneGeometry.h"

DECLARE_DIRTY_FUNC(CSkinnedMesh)(BasicComponent& bc)
{
	// Specifically for skinned mesh components (that need gpu uploads every frame)
	// we will use an external system instead of this dirty function for uploading the joints.
	// The same will also happen with the "animator" update.
	// Here we will just handle the rest of the updates (mesh changes etc)

	return [=, transform = bc.world().transform(), jointsLen = skinnedMesh.Lock()->joints.size()](
			   SceneAnimatedGeometry& geom) {
		XMStoreFloat4x4A(&geom.transform, transform);
		if constexpr (FullDirty) {
			geom.meshPod = skinnedMesh;
			geom.mesh = GpuAssetManager->GetGpuHandle(skinnedMesh);

			// Dirty-ness of joints length is checked on scene object side
			// geom.MaybeResizeJoints(jointsLen);
		}
	};
}
