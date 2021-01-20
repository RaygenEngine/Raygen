#include "SceneIrragrid.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/util/WriteDescriptorSets.h"


using namespace vl;

SceneIrragrid::SceneIrragrid()
	: SceneStruct(sizeof(Irragrid_UBO))
{
	environmentSamplerDescSet = Layouts->cubemapArray.AllocDescriptorSet();
	environmentStorageDescSet = Layouts->cubemapArrayStorage.AllocDescriptorSet();
	irradianceSamplerDescSet = Layouts->cubemapArray.AllocDescriptorSet();
	irradianceStorageDescSet = Layouts->cubemapArrayStorage.AllocDescriptorSet();
}

void SceneIrragrid::UploadUbo(uint32 curFrame)
{
	UploadDataToUbo(curFrame, &ubo, sizeof(Irragrid_UBO));
}

void SceneIrragrid::Allocate()
{
	Device->waitIdle();

	uint32 arraySize = ubo.width * ubo.height * ubo.depth;
	ubo.builtCount = arraySize;

	irradianceCubemaps = RCubemapArray(
		fmt::format("IrrCubes: CHECK:Irragrid"), irrResolution, vk::Format::eR32G32B32A32Sfloat, arraySize);

	environmentCubemaps = RCubemapArray(
		fmt::format("EnvCubes: CHECK:Irragrid"), irrResolution, vk::Format::eR32G32B32A32Sfloat, arraySize);

	rvk::writeDescriptorImages(environmentSamplerDescSet, 0u, { environmentCubemaps.view() });

	rvk::writeDescriptorImages(environmentStorageDescSet, 0u, { environmentCubemaps.view() },
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	rvk::writeDescriptorImages(irradianceSamplerDescSet, 0u, { irradianceCubemaps.view() });

	rvk::writeDescriptorImages(irradianceStorageDescSet, 0u, { irradianceCubemaps.view() },
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	shouldBuild.Set();
}
