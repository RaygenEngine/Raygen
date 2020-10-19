#include "SceneReflProbe.h"

#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/offline/BrdfLutCalculation.h"
#include "rendering/offline/IrradianceMapCalculation.h"
#include "rendering/offline/PathtracedCubemap.h"
#include "rendering/offline/PrefilteredMapCalculation.h"

#include "engine/Timer.h"


// WIP:
#include "rendering/Layer.h"
#include "rendering/scene/Scene.h"

void SceneReflProbe::Build()
{
	{
		TIMER_SCOPE("source")
		vl::PathtracedCubemap calcSourceSkybox(&envmap.Lock(), position, 1024);

		auto scene = vl::Layer->mainScene;
		calcSourceSkybox.Calculate(vl::Layer->mainScene->sceneAsDescSet, scene->tlas.sceneDesc.descSet[0],
			scene->tlas.sceneDesc.descSetPointlights[0], vl::Layer->mainScene->Get<ScenePointlight>().size());
	}

	{
		TIMER_SCOPE("irr")
		vl::IrradianceMapCalculation calcIrradiance(&envmap.Lock(), 32);
		calcIrradiance.Calculate();
	}

	{
		TIMER_SCOPE("pref")
		vl::PrefilteredMapCalculation calcPrefiltered(&envmap.Lock(), 128);
		calcPrefiltered.Calculate();
	}

	{
		TIMER_SCOPE("lut")
		vl::BrdfLutCalculation calcBrdfLut(&envmap.Lock(), 512);
		calcBrdfLut.Calculate();
	}
}
