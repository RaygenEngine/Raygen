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
	for (uint32 i = 0; i < 3; ++i) {
		descSets[i] = vl::Layouts->ambientDescLayout.GetDescriptorSet();
	}
}

void SceneReflectionProbe::UploadCubemap(PodHandle<Cubemap> cubemapData)
{
	cubemap = vl::GpuAssetManager->GetGpuHandle(cubemapData);

	auto& cubemapAsset = cubemap.Lock();

	auto quadSampler = vl::GpuAssetManager->GetDefaultSampler();


	vl::IrradianceMapCalculation calc(this);
}

void SceneReflectionProbe::Build() {}
