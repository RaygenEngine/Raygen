#include "GpuEnvironmentMap.h"

#include "assets/pods/EnvironmentMap.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuCubemap.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/Layouts.h"
#include "rendering/util/WriteDescriptorSets.h"

using namespace vl;

GpuEnvironmentMap::GpuEnvironmentMap(PodHandle<EnvironmentMap> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

void GpuEnvironmentMap::Update(const AssetUpdateInfo&)
{
	auto envmapPod = podHandle.Lock();
	ClearDependencies();
	AddDependencies(envmapPod->skybox, envmapPod->irradiance, envmapPod->prefiltered);

	skybox = GpuAssetManager->GetGpuHandle(envmapPod->skybox);

	// CHECK: is this needed?
	skybox.Lock().cubemap.GenerateMipmapsAndTransitionEach(
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	irradiance = GpuAssetManager->GetGpuHandle(envmapPod->irradiance);
	prefiltered = GpuAssetManager->GetGpuHandle(envmapPod->prefiltered);

	descriptorSet = Layouts->envmapLayout.AllocDescriptorSet();

	std::vector<vk::ImageView> cubemapsViews = {
		skybox.Lock().cubemap.view(),
		irradiance.Lock().cubemap.view(),
		prefiltered.Lock().cubemap.view(),
	};

	rvk::writeDescriptorImages(descriptorSet, 0u, std::move(cubemapsViews));
}
