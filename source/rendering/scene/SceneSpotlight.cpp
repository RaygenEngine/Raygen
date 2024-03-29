#include "SceneSpotlight.h"

#include "rendering/assets/GpuAssetManager.h"

void SceneSpotlight::MaybeResizeShadowmap(uint32 width, uint32 height)
{
	auto& extent = shadowmapPass.at(0).framebuffer.extent;

	if (width != extent.width || height != extent.height) {

		for (size_t i = 0; i < c_framesInFlight; ++i) {
			// TODO: this should be called once at the start (constructor - tidy scene structs)
			shadowmapDescSet[i] = vl::DescriptorLayouts->_1imageSampler.AllocDescriptorSet();

			shadowmapPass[i] = vl::PassLayouts->shadow.CreatePassInstance(width, height);

			depthSampler = vl::GpuAssetManager->GetShadow2dSampler();

			rvk::writeDescriptorImages(
				shadowmapDescSet[i], 0u, { shadowmapPass[i].framebuffer["Shadowmap"].view() }, depthSampler);
		}
	}
}
