#include "DirlightComponent.h"

DECLARE_DIRTY_FUNC(CDirlight)(BasicComponent& bc)
{
	const XMVECTOR lookAt = XMVectorAdd(bc.world().translation(), bc.world().front());
	const XMMATRIX view = XMMatrixLookAtRH(bc.world().translation(), lookAt, bc.world().up());
	const XMMATRIX proj = XMMatrixOrthographicOffCenterRH(left, right, bottom, top, near, far);

	return [=, name = bc.name, front = bc.world().front()](SceneDirlight& dl) {
		dl.name = "direct depth: " + name;
		XMStoreFloat3A(&dl.ubo.front, front);
		XMStoreFloat4x4A(&dl.ubo.viewProj, view * proj);

		if constexpr (FullDirty) {
			dl.ubo.color = { color.x, color.y, color.z };
			dl.ubo.intensity = intensity;

			dl.ubo.maxShadowBias = maxShadowBias;
			dl.ubo.samples = samples;
			dl.ubo.sampleInvSpread = 1 / radius;
			dl.ubo.hasShadow = hasShadow;
		}
	};
}
