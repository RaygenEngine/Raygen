#include "Depthmap.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/util/WriteDescriptorSets.h"

namespace vl {
Depthmap::Depthmap(uint32 width, uint32 height, const char* name)
{
	framebuffer.AddAttachment(width, height, Device->FindDepthFormat(), vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, name, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	framebuffer.Generate(Layouts->depthRenderPass.get());

	descSet = Layouts->singleSamplerDescLayout.AllocDescriptorSet();

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

	depthSampler = GpuResources::AcquireSampler(samplerInfo);

	rvk::writeDescriptorImages(descSet, 0u, { framebuffer[0].view() }, depthSampler);
}

Depthmap& Depthmap::operator=(Depthmap&& other)
{
	framebuffer = std::move(other.framebuffer);
	depthSampler = other.depthSampler;
	descSet = other.descSet;
	other.depthSampler = nullptr;

	return *this;
}

Depthmap::~Depthmap()
{
	// CHECK:
	if (depthSampler) {
		GpuResources::ReleaseSampler(depthSampler);
	}
}
} // namespace vl
