#include "SceneReflprobe.h"

#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/offline/BrdfLutCalculation.h"
#include "rendering/offline/IrradianceMapCalculation.h"
#include "rendering/offline/PathtracedCubemap.h"
#include "rendering/offline/PrefilteredMapCalculation.h"

#include "engine/Timer.h"


// WIP:
#include "rendering/Layer.h"
#include "rendering/scene/Scene.h"
#include "rendering/Device.h"

void SceneReflprobe::Build()
{
	vl::Device->waitIdle();

	TIMER_SCOPE("pt + irr + pref")

	vl::PathtracedCubemap calcSourceSkybox(&envmap.Lock(), ubo.position, 256);

	auto scene = vl::Layer->mainScene;
	calcSourceSkybox.Calculate(vl::Layer->mainScene->sceneAsDescSet, scene->tlas.sceneDesc.descSet[0],
		scene->tlas.sceneDesc.descSetPointlights[0],
		static_cast<int32>(vl::Layer->mainScene->Get<ScenePointlight>().size()), this);

	vl::IrradianceMapCalculation calcIrradiance(&envmap.Lock(), 32);
	calcIrradiance.Calculate();


	vl::PrefilteredMapCalculation calcPrefiltered(&envmap.Lock(), 256);
	calcPrefiltered.Calculate();

	vl::Device->waitIdle();
}
