#include "SceneIrragrid.h"

#include "rendering/Device.h"

using namespace vl;

SceneIrragrid::SceneIrragrid()
	: SceneStruct(sizeof(Irragrid_UBO))
{
	environmentSamplerDescSet = DescriptorLayouts->_1imageSampler.AllocDescriptorSet();
	irradianceSamplerDescSet = DescriptorLayouts->_1imageSampler.AllocDescriptorSet();
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

	if (arraySize * 6u > 2048) {
		LOG_ERROR("Reduce Irragrid's count of probes");
		return;
	}

	irradianceCubemaps = RCubemapArray(
		fmt::format("IrrCubes: CHECK:Irragrid"), irrResolution, vk::Format::eR32G32B32A32Sfloat, arraySize);

	environmentCubemaps = RCubemapArray(
		fmt::format("EnvCubes: CHECK:Irragrid"), irrResolution, vk::Format::eR32G32B32A32Sfloat, arraySize);

	rvk::writeDescriptorImages(environmentSamplerDescSet, 0u, { environmentCubemaps.view() });

	rvk::writeDescriptorImages(irradianceSamplerDescSet, 0u, { irradianceCubemaps.view() });

	shouldBuild.Set();
}
