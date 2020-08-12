#include "pch.h"
#include "universe/components/CameraComponent.h"

#include "rendering/scene/SceneCamera.h"

DECLARE_DIRTY_FUNC(CCamera)(BasicComponent& bc)
{
	auto lookAt = bc.world().position + bc.world().forward() * focalLength;
	auto view = glm::lookAt(bc.world().position, lookAt, bc.world().up());

	auto viewInv = glm::inverse(view);

	[[maybe_unused]] glm::mat4 proj;
	[[maybe_unused]] glm::mat4 projInv;
	[[maybe_unused]] glm::mat4 viewProj;
	[[maybe_unused]] glm::mat4 viewProjInv;

	if constexpr (FullDirty) {
		const auto ar = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
		hFov = 2 * atan(ar * tan(vFov * 0.5f));

		const auto top = tan(vFov / 2.f + vFovOffset) * near;
		const auto bottom = tan(-vFov / 2.f - vFovOffset) * near;

		const auto right = tan(hFov / 2.f + hFovOffset) * near;
		const auto left = tan(-hFov / 2.f - hFovOffset) * near;

		proj = glm::frustum(left, right, bottom, top, near, far);
		// Vulkan's inverted y
		proj[1][1] *= -1.f;

		projInv = glm::inverse(proj);
		viewProj = proj * view;
		viewProjInv = glm::inverse(viewProj);
	}

	return [=](SceneCamera& cam) {
		cam.ubo.position = glm::vec4(bc.world().position, 1.f);
		cam.ubo.view = view;
		cam.ubo.viewInv = viewInv;

		if constexpr (FullDirty) {
			cam.ubo.proj = proj;
			cam.ubo.projInv = projInv;
			cam.ubo.viewProj = viewProj;
			cam.ubo.viewProjInv = viewProjInv;
		}
	};
}
