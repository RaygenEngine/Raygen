#include "pch.h"
#include "SceneReflectionProbe.h"

#include "assets/pods/EnvironmentMap.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Layouts.h"
#include "rendering/passes/IrradianceMapCalculation.h"
#include "rendering/Renderer.h"

void SceneReflectionProbe::Build()
{
	vl::IrradianceMapCalculation calc(&envmap.Lock(), 32);
	calc.Calculate();
}
