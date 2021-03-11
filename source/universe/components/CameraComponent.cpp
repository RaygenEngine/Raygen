#include "universe/components/CameraComponent.h"

#include "rendering/scene/SceneCamera.h"

DECLARE_DIRTY_FUNC(CCamera)(BasicComponent& bc)
{
	const XMVECTOR lookAt = XMVectorAdd(bc.world().translation(), XMVectorScale(bc.world().front(), focalLength));
	pData->view = XMMatrixLookAtRH(bc.world().translation(), lookAt, bc.world().up());

	// TODO: avoid calculating every frame?
	const auto ar = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	hFov = 2 * atan(ar * tan(vFov * 0.5f));

	const auto top = tan(vFov / 2.f + vFovOffset) * near;
	const auto bottom = tan(-vFov / 2.f - vFovOffset) * near;

	const auto right = tan(hFov / 2.f + hFovOffset) * near;
	const auto left = tan(-hFov / 2.f - hFovOffset) * near;


	pData->proj = XMMatrixPerspectiveOffCenterRH(left, right, bottom, top, near, far);

	const XMMATRIX _projInv = XMMatrixInverse(nullptr, pData->proj);
	const XMMATRIX _viewInv = XMMatrixInverse(nullptr, pData->view);

	return [=, position = bc.world().translation()](SceneCamera& cam) {
		cam.prevViewProj = cam.ubo.viewProj;
		XMStoreFloat3A(&cam.ubo.position, position);
		XMStoreFloat4x4A(&cam.ubo.view, pData->view);
		XMStoreFloat4x4A(&cam.ubo.proj, pData->proj);
		XMStoreFloat4x4A(&cam.ubo.viewProj, XMMatrixMultiply(pData->view, pData->proj));

		XMStoreFloat4x4A(&cam.ubo.viewInv, _viewInv);
		XMStoreFloat4x4A(&cam.ubo.projInv, _projInv);
		XMStoreFloat4x4A(&cam.ubo.viewProjInv, XMMatrixMultiply(_projInv, _viewInv));
	};
}

CCamera::CCamera()
{
	pData = (AlignedData*)_aligned_malloc(sizeof(AlignedData), 16);
}

CCamera::~CCamera()
{
	_aligned_free(pData);
}
