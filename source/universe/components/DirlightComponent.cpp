#include "DirlightComponent.h"

#include "assets/PodEditor.h"
#include "assets/pods/MaterialArchetype.h"
#include "assets/pods/MaterialInstance.h"
#include "rendering/scene/SceneDirlight.h"


DECLARE_DIRTY_FUNC(CDirlight)(BasicComponent& bc)
{
	const XMVECTOR lookAt = XMVectorAdd(bc.world().translation(), bc.world().front());
	pData->view = XMMatrixLookAtRH(bc.world().translation(), lookAt, bc.world().up());
	pData->proj = XMMatrixOrthographicOffCenterRH(left, right, bottom, top, near, far);

	return [=, front = bc.world().front()](SceneDirlight& dl) {
		dl.name = "direct depth: " + bc.name;
		XMStoreFloat3A(&dl.ubo.front, front);
		XMStoreFloat4x4A(&dl.ubo.viewProj, XMMatrixMultiply(pData->view, pData->proj));

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

CDirlight::CDirlight()
{
	pData = (AlignedData*)_aligned_malloc(sizeof(AlignedData), 16);
}

CDirlight::~CDirlight()
{
	_aligned_free(pData);
}
