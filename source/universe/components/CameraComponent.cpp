#include "universe/components/CameraComponent.h"

#include "rendering/scene/SceneCamera.h"

DECLARE_DIRTY_FUNC(CCamera)(BasicComponent& bc)
{
	auto lookAt = bc.world().position + bc.world().front() * focalLength;
	view = glm::lookAt(bc.world().position, lookAt, bc.world().up());

	auto viewInv = glm::inverse(view);

	const auto ar = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	hFov = 2 * atan(ar * tan(vFov * 0.5f));

	const auto top = tan(vFov / 2.f + vFovOffset) * near;
	const auto bottom = tan(-vFov / 2.f - vFovOffset) * near;

	const auto right = tan(hFov / 2.f + hFovOffset) * near;
	const auto left = tan(-hFov / 2.f - hFovOffset) * near;

	proj = glm::frustum(left, right, bottom, top, near, far);
	// Vulkan's inverted y
	proj[1][1] *= -1.f;

	auto projInv = glm::inverse(proj);
	auto viewProj = proj * view;
	auto viewProjInv = viewInv * projInv;

	return [=, position = bc.world().position](SceneCamera& cam) {
		cam.prevViewProj = cam.ubo.viewProj;
		cam.ubo.position = glm::vec4(position, 1.f);
		cam.ubo.view = view;
		cam.ubo.viewInv = viewInv;

		cam.ubo.proj = proj;
		cam.ubo.projInv = projInv;
		cam.ubo.viewProj = viewProj;
		cam.ubo.viewProjInv = viewProjInv;
	};
}
