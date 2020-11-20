#include "SceneIrragrid.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/util/WriteDescriptorSets.h"


using namespace vl;

SceneIrragrid::SceneIrragrid()
	: SceneStruct(sizeof(decltype(ubo)))
{
	environmentSamplerDescSet = Layouts->cubemapArray.AllocDescriptorSet();
	environmentStorageDescSet = Layouts->cubemapArrayStorage.AllocDescriptorSet();
	irradianceSamplerDescSet = Layouts->cubemapArray.AllocDescriptorSet();
	irradianceStorageDescSet = Layouts->cubemapArrayStorage.AllocDescriptorSet();
}

void SceneIrragrid::Allocate()
{
	Device->waitIdle();

	uint32 arraySize = ubo.width * ubo.height * ubo.depth;
	ubo.builtCount = arraySize;

	irradianceCubemaps = RCubemapArray(resolution, 1u, arraySize, vk::Format::eR32G32B32A32Sfloat,
		vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("IrrCubes: CHECK:Irragrid"));

	environmentCubemaps
		= RCubemapArray(resolution, 1u, arraySize, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
			vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("EnvCubes: CHECK:Irragrid"));

	rvk::writeDescriptorImages(environmentSamplerDescSet, 0u, { environmentCubemaps.view() });

	rvk::writeDescriptorImages(environmentStorageDescSet, 0u, { environmentCubemaps.view() },
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	rvk::writeDescriptorImages(irradianceSamplerDescSet, 0u, { irradianceCubemaps.view() });

	rvk::writeDescriptorImages(irradianceStorageDescSet, 0u, { irradianceCubemaps.view() },
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	shouldBuild.Set();
}
