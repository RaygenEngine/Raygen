#include "universe/components/CameraComponent.h"

DECLARE_DIRTY_FUNC(CCamera)(BasicComponent& bc)
{
	const XMVECTOR lookAt = XMVectorAdd(bc.world().translation(), XMVectorScale(bc.world().front(), focalLength));
	const XMMATRIX view = XMMatrixLookAtRH(bc.world().translation(), lookAt, bc.world().up());

	// TODO: avoid calculating every frame?
	const auto ar = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	hFov = 2 * atan(ar * tan(vFov * 0.5f));

	const auto top = tan(vFov / 2.f + vFovOffset) * near;
	const auto bottom = tan(-vFov / 2.f - vFovOffset) * near;

	const auto right = tan(hFov / 2.f + hFovOffset) * near;
	const auto left = tan(-hFov / 2.f - hFovOffset) * near;


	const XMMATRIX proj = XMMatrixPerspectiveOffCenterRH(left, right, bottom, top, near, far);

	const XMMATRIX projInv = XMMatrixInverse(nullptr, proj);
	const XMMATRIX viewInv = XMMatrixInverse(nullptr, view);

	return [=, position = bc.world().translation()](SceneCamera& cam) {
		cam.prevViewProj = cam.ubo.viewProj;
		XMStoreFloat3A(&cam.ubo.position, position);
		XMStoreFloat4x4A(&cam.ubo.view, view);
		XMStoreFloat4x4A(&cam.ubo.proj, proj);
		XMStoreFloat4x4A(&cam.ubo.viewProj, view * proj);

		XMStoreFloat4x4A(&cam.ubo.viewInv, viewInv);
		XMStoreFloat4x4A(&cam.ubo.projInv, projInv);
		XMStoreFloat4x4A(&cam.ubo.viewProjInv, projInv * viewInv);
	};
}
