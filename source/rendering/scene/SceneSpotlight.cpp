#include "SceneSpotlight.h"

#include "rendering/Layouts.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/util/WriteDescriptorSets.h"

void SceneSpotlight::MaybeResizeShadowmap(uint32 width, uint32 height)
{
	auto& extent = shadowmapPass.at(0).framebuffer.extent;

	if (width != extent.width || height != extent.height) {

		for (size_t i = 0; i < c_framesInFlight; ++i) {
			// TODO: this should be called once at the start (constructor - tidy scene structs)
			shadowmapDescSet[i] = vl::Layouts->singleSamplerDescLayout.AllocDescriptorSet();

			shadowmapPass[i] = vl::Layouts->shadowPassLayout.CreatePassInstance(width, height);

			// sampler2DShadow
			vk::SamplerCreateInfo samplerInfo{};
			samplerInfo
				.setMagFilter(vk::Filter::eLinear) //
				.setMinFilter(vk::Filter::eLinear)
				.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
				.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
				.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
				.setAnisotropyEnable(VK_FALSE)
				.setMaxAnisotropy(1u)
				.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
				.setUnnormalizedCoordinates(VK_FALSE)
				.setCompareEnable(VK_TRUE)
				.setCompareOp(vk::CompareOp::eLess)
				.setMipmapMode(vk::SamplerMipmapMode::eNearest)
				.setMipLodBias(0.f)
				.setMinLod(0.f)
				.setMaxLod(32.f);

			depthSampler = vl::GpuResources::AcquireSampler(samplerInfo);

			rvk::writeDescriptorImages(
				shadowmapDescSet[i], 0u, { shadowmapPass[i].framebuffer[0].view() }, depthSampler);
		}
	}
}
