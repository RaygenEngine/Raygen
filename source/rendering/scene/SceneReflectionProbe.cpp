#include "pch.h"
#include "SceneReflectionProbe.h"

#include "assets/pods/Cubemap.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Layouts.h"
#include "rendering/passes/IrradianceMapCalculation.h"
#include "rendering/Renderer.h"

SceneReflectionProbe::SceneReflectionProbe()
	: SceneStruct<Ambient_Ubo>()
{
}

void SceneReflectionProbe::Build()
{
	vl::IrradianceMapCalculation calc(&cubemap.Lock(), irradianceMapResolution);
	calc.Calculate();
}
