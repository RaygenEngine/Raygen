#include "pch.h"
#include "SceneReflectionProbe.h"

#include "assets/pods/EnvironmentMap.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Layouts.h"
#include "rendering/passes/IrradianceMapCalculation.h"
#include "rendering/passes/PrefilteredMapCalculation.h"
#include "rendering/passes/BrdfLutCalculation.h"
#include "rendering/Renderer.h"

void SceneReflectionProbe::Build()
{
	vl::IrradianceMapCalculation calc(&envmap.Lock(), 32);
	calc.Calculate();

	vl::PrefilteredMapCalculation calc1(&envmap.Lock(), 128);
	calc1.Calculate();

	vl::BrdfLutCalculation calc2(&envmap.Lock(), 512);
	calc2.Calculate();
}
