#include "Depthmap.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/resource/GpuResources.h"

namespace vl {
Depthmap::Depthmap(uint32 width, uint32 height, const char* name)
{
	framebuffer.AddAttachment(width, height, Device->FindDepthFormat(), vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, name, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	framebuffer.Generate(Layouts->depthRenderPass.get());

	// description set
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

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(framebuffer[0].view())
		.setSampler(depthSampler);

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(descSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setImageInfo(imageInfo);

	Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
}

// Depthmap::~Depthmap()
//{
//	 GpuResources::ReleaseSampler(depthSampler);
//}
} // namespace vl
