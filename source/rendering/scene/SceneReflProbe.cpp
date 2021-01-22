#include "SceneReflprobe.h"

#include "rendering/scene/Scene.h"

using namespace vl;

SceneReflprobe::SceneReflprobe()
	: SceneStruct(sizeof(Reflprobe_UBO))
{
	environmentSamplerDescSet = Layouts->singleSamplerDescLayout.AllocDescriptorSet();
	irradianceSamplerDescSet = Layouts->singleSamplerDescLayout.AllocDescriptorSet();
	prefilteredSamplerDescSet = Layouts->singleSamplerDescLayout.AllocDescriptorSet();
}

void SceneReflprobe::Allocate()
{
	constexpr uint32 maxLods = 10;

	CLOG_ABORT(ubo.lodCount > maxLods, "Lod count for reflprobe exceeds maximum lod count of {}", maxLods);

	// resolution >= 32 CHECK:
	uint32 resolution = static_cast<uint32>(std::pow(2, ubo.lodCount + 5));

	Device->waitIdle();

	environment = RCubemap(fmt::format("Reflprobe-environment"), resolution, vk::Format::eR32G32B32A32Sfloat);

	irradiance = RCubemap(fmt::format("Reflprobe-irradiance"), irrResolution, vk::Format::eR32G32B32A32Sfloat);

	prefiltered
		= RCubemap(fmt::format("Reflprobe-prefiltered"), resolution, vk::Format::eR32G32B32A32Sfloat, ubo.lodCount);

	rvk::writeDescriptorImages(environmentSamplerDescSet, 0u, { environment.view() });
	rvk::writeDescriptorImages(irradianceSamplerDescSet, 0u, { irradiance.view() });
	rvk::writeDescriptorImages(prefilteredSamplerDescSet, 0u, { prefiltered.view() });

	shouldBuild.Set();
}
