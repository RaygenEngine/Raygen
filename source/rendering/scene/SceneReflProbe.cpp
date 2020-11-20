#include "SceneReflprobe.h"

#include "rendering/scene/Scene.h"
#include "rendering/util/WriteDescriptorSets.h"

using namespace vl;

SceneReflprobe::SceneReflprobe()
	: SceneStruct(sizeof(Reflprobe_UBO))
{
	environmentSamplerDescSet = Layouts->singleSamplerDescLayout.AllocDescriptorSet();
	environmentStorageDescSet = Layouts->singleStorageImage.AllocDescriptorSet();
	irradianceSamplerDescSet = Layouts->singleSamplerDescLayout.AllocDescriptorSet();
	irradianceStorageDescSet = Layouts->singleStorageImage.AllocDescriptorSet();
	prefilteredSamplerDescSet = Layouts->singleSamplerDescLayout.AllocDescriptorSet();
	prefilteredStorageDescSet = Layouts->storageImageArray10.AllocDescriptorSet();
}

void SceneReflprobe::Allocate()
{
	constexpr uint32 maxLods = 10;

	CLOG_ABORT(ubo.lodCount > maxLods, "Lod count for reflprobe exceeds maximum lod count of {}", maxLods);

	// resolution >= 32 WIP:
	uint32 resolution = static_cast<uint32>(std::pow(2, ubo.lodCount + 5));

	Device->waitIdle();

	environment = RCubemap(resolution, 1u, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("SurrCube: CHECK:reflprobenamehere"));

	irradiance = RCubemap(irrResolution, 1u, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("IrrCube: CHECK:reflprobenamehere"));

	prefiltered = RCubemap(resolution, ubo.lodCount, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("PreCube: CHECK:reflprobenamehere"));

	rvk::writeDescriptorImages(environmentSamplerDescSet, 0u, { environment.view() });
	rvk::writeDescriptorImages(irradianceSamplerDescSet, 0u, { irradiance.view() });
	rvk::writeDescriptorImages(prefilteredSamplerDescSet, 0u, { prefiltered.view() });

	rvk::writeDescriptorImages(environmentStorageDescSet, 0u, { environment.view() }, vk::DescriptorType::eStorageImage,
		vk::ImageLayout::eGeneral);
	rvk::writeDescriptorImages(irradianceStorageDescSet, 0u, { irradiance.view() }, vk::DescriptorType::eStorageImage,
		vk::ImageLayout::eGeneral);

	prefilteredMipViews = prefiltered.GetMipViews();

	std::vector<vk::ImageView> views{ maxLods };
	std::fill_n(views.data(), maxLods, prefilteredMipViews[0].get());
	std::copy_n(vk::uniqueToRaw(prefilteredMipViews).data(), prefilteredMipViews.size(), views.data());

	rvk::writeDescriptorImageArray(prefilteredStorageDescSet, 0u, std::move(views), nullptr,
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	shouldBuild.Set();
}
