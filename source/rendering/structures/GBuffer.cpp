#include "pch.h"
#include "GBuffer.h"

#include "engine/console/ConsoleVariable.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"

ConsoleVariable<int32> console_rCullMode("r.culling", static_cast<int32>(vk::CullModeFlagBits::eBack));

namespace vl {
GBuffer::GBuffer(uint32 width, uint32 height)
{
	for (size_t i = 0; i < ColorAttachmentCount; ++i) {
		framebuffer.AddAttachment(width, height, colorAttachmentFormats[i], vk::ImageTiling::eOptimal,
			vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal, attachmentNames[i], vk::ImageLayout::eColorAttachmentOptimal);
	}

	framebuffer.AddAttachment(width, height, Device->FindDepthFormat(), vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, attachmentNames[GDepth],
		vk::ImageLayout::eDepthStencilAttachmentOptimal);


	framebuffer.Generate(Layouts->gbufferPass.get());


	descSet = Layouts->gbufferDescLayout.AllocDescriptorSet();

	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	// PERF: Use single update
	for (uint32 i = 0; i < GCount; ++i) {
		vk::DescriptorImageInfo imageInfo{};
		imageInfo
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(framebuffer[i].view())
			.setSampler(quadSampler);

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite
			.setDstSet(descSet) //
			.setDstBinding(i)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setImageInfo(imageInfo);

		Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
}
} // namespace vl
