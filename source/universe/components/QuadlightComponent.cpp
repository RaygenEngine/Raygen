#include "QuadlightComponent.h"

DECLARE_DIRTY_FUNC(CQuadlight)(BasicComponent& bc)
{
	return [=, center = bc.world().translation(), front = bc.world().front(), up = bc.world().up(),
			   right = bc.world().right(), scale = bc.world().scale(),
			   orientation = bc.world().orientation()](SceneQuadlight& ql) {
		XMStoreFloat3A(&ql.ubo.center, center);
		XMStoreFloat4A(&ql.ubo.orienation, orientation); // NEW:: width, height
		// XMStoreFloat4x4A(&ql.transform, XMMatrixMultiply(pData->view, pData->proj));

		// ql.transform = math::transformMat(glm::vec3{ width, height, 1.f }, orientation, center);

		if constexpr (FullDirty) {
			ql.ubo.color = { color.x, color.y, color.z };
			ql.ubo.cosAperture = glm::cos(aperture / 2.f);
			ql.ubo.intensity = intensity;
		}
	};
}
