#include "pch.h"
#include "SceneReflProbe.h"

#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/offline/BrdfLutCalculation.h"
#include "rendering/offline/IrradianceMapCalculation.h"
#include "rendering/offline/PrefilteredMapCalculation.h"

void SceneReflProbe::Build()
{
	vl::IrradianceMapCalculation calc(&envmap.Lock(), 32);
	calc.Calculate();

	vl::PrefilteredMapCalculation calc1(&envmap.Lock(), 128);
	calc1.Calculate();

	vl::BrdfLutCalculation calc2(&envmap.Lock(), 512);
	calc2.Calculate();
}
