#include "pch.h"
#include "DeferredPass.h"

#include "assets/Assets.h"
#include "engine/Engine.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/renderer/Renderer.h"
#include "rendering/scene/Scene.h"

namespace vl {
std::pair<DescriptorLayout&, vk::DescriptorSet&> PrepareGBufferDescription(
	DescriptorLayout& gBufferDescLayout, vk::DescriptorSet& gBufferDescSet)
{
	for (uint32 i = 0u; i < 6u; ++i) {
		gBufferDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	}
	gBufferDescLayout.Generate();

	gBufferDescSet = gBufferDescLayout.GetDescriptorSet();

	return { gBufferDescLayout, gBufferDescSet };
}

DeferredPass::DeferredPass(vk::RenderPass renderPass)
	: m_slPass(renderPass, PrepareGBufferDescription(m_gBufferDescLayout, m_gBufferDescSet))
{
}

void DeferredPass::RecordCmd(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);

	m_slPass.RecordCmd(cmdBuffer, GetViewport(), GetScissor());
}

void DeferredPass::UpdateGBufferDescriptorSet(GBuffer& gbuffer)
{
	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	for (uint32 i = 0; i < GCount; ++i) {

		vk::DescriptorImageInfo imageInfo{};
		imageInfo
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(gbuffer[i]->GetView())
			.setSampler(quadSampler);

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite
			.setDstSet(m_gBufferDescSet) //
			.setDstBinding(i)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1u)
			.setPBufferInfo(nullptr)
			.setPImageInfo(&imageInfo)
			.setPTexelBufferView(nullptr);

		Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
}

vk::Viewport DeferredPass::GetViewport() const
{
	auto& rect = Renderer->GetViewportRect();
	const float x = static_cast<float>(rect.offset.x);
	const float y = static_cast<float>(rect.offset.y);
	const float width = static_cast<float>(rect.extent.width);
	const float height = static_cast<float>(rect.extent.height);

	vk::Viewport viewport{};
	viewport
		.setX(x) //
		.setY(y)
		.setWidth(width)
		.setHeight(height)
		.setMinDepth(0.f)
		.setMaxDepth(1.f);

	return viewport;
}

vk::Rect2D DeferredPass::GetScissor() const
{
	return Renderer->GetViewportRect();
}
} // namespace vl
