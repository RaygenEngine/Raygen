#include "pch.h"
#include "RDepthmap.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/VulkanUtl.h"

namespace vl {
RDepthmap::RDepthmap(uint32 width, uint32 height, const char* name)
{
	// attachment
	vk::Format depthFormat = Device->FindDepthFormat();

	attachment = RImageAttachment{ width, height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, name };

	attachment.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	// framebuffer
	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(Layouts->depthRenderPass.get()) //
		.setAttachmentCount(1u)
		.setPAttachments(&vk::ImageView(attachment))
		.setWidth(width)
		.setHeight(height)
		.setLayers(1);

	framebuffer = Device->createFramebufferUnique(createInfo);

	// description set
	descSet = Layouts->singleSamplerDescLayout.GetDescriptorSet();

	// sampler
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

	depthSampler = Device->createSamplerUnique(samplerInfo);

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(attachment)
		.setSampler(depthSampler.get());

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(descSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1u)
		.setPBufferInfo(nullptr)
		.setPImageInfo(&imageInfo)
		.setPTexelBufferView(nullptr);

	Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
}
} // namespace vl
