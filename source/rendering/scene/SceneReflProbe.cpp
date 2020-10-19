#include "SceneReflProbe.h"

#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/offline/BrdfLutCalculation.h"
#include "rendering/offline/IrradianceMapCalculation.h"
#include "rendering/offline/PathtracedCubemap.h"
#include "rendering/offline/PrefilteredMapCalculation.h"


// WIP:
#include "rendering/Layer.h"
#include "rendering/scene/Scene.h"

void SceneReflProbe::Build()
{
	vl::PathtracedCubemap calcSourceSkybox(&envmap.Lock(), position, 512);

	auto scene = vl::Layer->mainScene;
	calcSourceSkybox.Calculate(vl::Layer->mainScene->sceneAsDescSet, scene->tlas.sceneDesc.descSet[0],
		scene->tlas.sceneDesc.descSetSpotlights[0]);

	vl::IrradianceMapCalculation calcIrradiance(&envmap.Lock(), 32);
	calcIrradiance.Calculate();

	vl::PrefilteredMapCalculation calcPrefiltered(&envmap.Lock(), 512);
	calcPrefiltered.Calculate();

	vl::BrdfLutCalculation calcBrdfLut(&envmap.Lock(), 512);
	calcBrdfLut.Calculate();
}
