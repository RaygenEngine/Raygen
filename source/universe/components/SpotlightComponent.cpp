#include "SpotlightComponent.h"

DECLARE_DIRTY_FUNC(CSpotlight)(BasicComponent& bc)
{
	const XMVECTOR lookAt = XMVectorAdd(bc.world().translation(), bc.world().front());
	const XMMATRIX view = XMMatrixLookAtRH(bc.world().translation(), lookAt, bc.world().up());

	float outerCutOff = glm::cos(outerAperture / 2.f);
	float innerCutOff = glm::cos(innerAperture / 2.f);

	const XMMATRIX proj = XMMatrixPerspectiveRH(shadowMapWidth, shadowMapHeight, near, far);

	return [=, name = bc.name, translation = bc.world().translation(), front = bc.world().front()](SceneSpotlight& sl) {
		sl.name = "spot depth: " + name;
		XMStoreFloat3A(&sl.ubo.position, translation);
		XMStoreFloat3A(&sl.ubo.front, front);
		XMStoreFloat4x4A(&sl.ubo.viewProj, view * proj);

		if constexpr (FullDirty) {
			sl.ubo.color = { color.x, color.y, color.z };
			sl.ubo.intensity = intensity;
			sl.ubo.near = near;
			sl.ubo.far = far;
			sl.ubo.outerCutOff = outerCutOff;
			sl.ubo.innerCutOff = innerCutOff;
			sl.ubo.constantTerm = constantTerm;
			sl.ubo.linearTerm = linearTerm;
			sl.ubo.quadraticTerm = quadraticTerm;
			sl.ubo.maxShadowBias = maxShadowBias;
			sl.ubo.samples = samples;
			sl.ubo.sampleInvSpread = 1 / radius;
			sl.ubo.hasShadow = hasShadow;
		}
	};
}
