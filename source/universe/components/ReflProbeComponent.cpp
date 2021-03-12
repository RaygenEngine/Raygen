#include "ReflProbeComponent.h"

DECLARE_DIRTY_FUNC(CReflprobe)(BasicComponent& bc)
{
	return [=, translation = bc.world().translation()](SceneReflprobe& rp) {
		XMStoreFloat3A(&rp.ubo.position, translation);

		if (FullDirty) {

			rp.ubo.radius = radius;

			rp.ptSamples = ptSamples;
			rp.ptBounces = ptBounces;

			rp.irrResolution = irrResolution;

			rp.ubo.lodCount = prefLodCount;
			rp.prefSamples = prefSamples;

			rp.ubo.irradianceFactor = applyIrradiance;
		}
	};
}
