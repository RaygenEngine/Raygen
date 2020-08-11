#include "pch.h"
#include "Renderer.h"

#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/passes/DepthmapPass.h"
#include "rendering/passes/GBufferPass.h"
#include "rendering/passes/UnlitPass.h"
#include "rendering/ppt/techniques/PtDebug.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneDirectionalLight.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/VulkanUtl.h"
#include "rendering/wrappers/RDepthmap.h"

#include <editor/imgui/ImguiImpl.h>

namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {

Renderer_::Renderer_()
{
	Event::OnViewportUpdated.BindFlag(this, m_didViewportResize);


	vk::AttachmentDescription depthAttachmentDesc{};
	depthAttachmentDesc
		.setFormat(Device->FindDepthFormat()) //
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eLoad)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef
		.setAttachment(2u) //
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	// post process render pass

	vk::AttachmentDescription colorAttachmentDesc{};
	vk::AttachmentReference colorAttachmentRef{};

	colorAttachmentDesc.setFormat(vk::Format::eR32G32B32A32Sfloat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	colorAttachmentRef
		.setAttachment(0u) //
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference colorAttachmentRef15{};

	colorAttachmentRef15
		.setAttachment(0u) //
		.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);


	vk::AttachmentDescription colorAttachmentDesc2{};
	vk::AttachmentReference colorAttachmentRef2{};

	colorAttachmentDesc2.setFormat(vk::Format::eR32G32B32A32Sfloat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	colorAttachmentRef2
		.setAttachment(1u) //
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);


	vk::SubpassDescription lightSubpass{};
	lightSubpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachmentCount(1u)
		.setPColorAttachments(&colorAttachmentRef)
		.setPDepthStencilAttachment(nullptr);

	vk::SubpassDependency lightDep{};
	lightDep
		.setSrcSubpass(VK_SUBPASS_EXTERNAL) //
		.setDstSubpass(0u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
		.setDependencyFlags(vk::DependencyFlagBits::eByRegion);


	vk::SubpassDescription debugSupass{};
	debugSupass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachmentCount(1u)
		.setInputAttachmentCount(1u)
		.setPDepthStencilAttachment(&depthAttachmentRef)
		.setPInputAttachments(&colorAttachmentRef15)
		.setPColorAttachments(&colorAttachmentRef2);

	vk::SubpassDependency debugDep{};
	debugDep
		.setSrcSubpass(0u) //
		.setDstSubpass(1u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
		.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	std::array subpasses{ lightSubpass, debugSupass };
	std::array dependcies{ lightDep, debugDep };
	std::array attachments{ colorAttachmentDesc, colorAttachmentDesc2, depthAttachmentDesc };


	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachmentCount(static_cast<uint32>(attachments.size())) //
		.setPAttachments(attachments.data())
		.setSubpassCount(static_cast<uint32>(subpasses.size()))
		.setPSubpasses(subpasses.data())
		.setDependencyCount(static_cast<uint32>(dependcies.size()))
		.setPDependencies(dependcies.data());

	m_ptRenderpass = Device->createRenderPassUnique(renderPassInfo);

	// descsets
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_ppDescSets[i] = Layouts->singleSamplerDescLayout.GetDescriptorSet();
	}
}

void Renderer_::InitPipelines(vk::RenderPass outRp)
{
	m_copyHdrTexture.MakePipeline(outRp);
	m_postprocCollection.RegisterTechniques();
}

void Renderer_::RecordGeometryPasses(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	auto extent = m_gbuffer->attachments[GNormal]->GetExtent2D();

	vk::Rect2D scissor{};
	scissor
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	vk::Viewport viewport{};
	viewport
		.setX(0) //
		.setY(0)
		.setWidth(static_cast<float>(extent.width))
		.setHeight(static_cast<float>(extent.height))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);


	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(Layouts->gbufferPass.get()) //
		.setFramebuffer(m_gbuffer->framebuffer.get());
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	std::array<vk::ClearValue, 6> clearValues = {};
	clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[1].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[2].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[3].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[4].setDepthStencil({ 1.0f, 0 });
	renderPassInfo
		.setClearValueCount(static_cast<uint32>(clearValues.size())) //
		.setPClearValues(clearValues.data());


	cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
	{
		auto buffer = m_secondaryBuffersPool.Get(sceneDesc.frameIndex);

		GbufferPass::RecordCmd(&buffer, viewport, scissor, sceneDesc);

		cmdBuffer->executeCommands({ buffer });
	}
	cmdBuffer->endRenderPass();

	auto shadowmapRenderpass = [&](auto light) {
		if (light) {
			auto extent = light->shadowmap->attachment->GetExtent2D();

			vk::Rect2D scissor{};

			scissor
				.setOffset({ 0, 0 }) //
				.setExtent(extent);

			auto vpSize = extent;

			vk::Viewport viewport{};
			viewport
				.setX(0) //
				.setY(0)
				.setWidth(static_cast<float>(vpSize.width))
				.setHeight(static_cast<float>(vpSize.height))
				.setMinDepth(0.f)
				.setMaxDepth(1.f);

			vk::RenderPassBeginInfo renderPassInfo{};
			renderPassInfo
				.setRenderPass(Layouts->depthRenderPass.get()) //
				.setFramebuffer(light->shadowmap->framebuffer.get());
			renderPassInfo.renderArea
				.setOffset({ 0, 0 }) //
				.setExtent(extent);

			vk::ClearValue clearValues = {};
			clearValues.setDepthStencil({ 1.0f, 0 });
			renderPassInfo
				.setClearValueCount(1u) //
				.setPClearValues(&clearValues);

			cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
			{
				auto buffer = m_secondaryBuffersPool.Get(sceneDesc.frameIndex);

				DepthmapPass::RecordCmd(&buffer, viewport, scissor, light->ubo.viewProj, sceneDesc);

				cmdBuffer->executeCommands({ buffer });
			}
			cmdBuffer->endRenderPass();
		}
	};


	for (auto sl : sceneDesc->spotlights.elements) {
		shadowmapRenderpass(sl);
	}

	for (auto dl : sceneDesc->directionalLights.elements) {
		shadowmapRenderpass(dl);
	}
}

void Renderer_::RecordPostProcessPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	auto extent = m_gbuffer->attachments[GNormal]->GetExtent2D();

	vk::Rect2D scissor{};
	scissor
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	vk::Viewport viewport{};
	viewport
		.setX(0) //
		.setY(0)
		.setWidth(static_cast<float>(extent.width))
		.setHeight(static_cast<float>(extent.height))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);


	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(m_ptRenderpass.get()) //
		.setFramebuffer(m_framebuffers[sceneDesc.frameIndex].get());
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	vk::ClearValue clearValue{};
	clearValue.setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });

	vk::ClearValue clearValue2{};
	clearValue2.setColor(std::array{ 0.2f, 0.2f, 0.0f, 1.0f });

	std::array cv{ clearValue, clearValue2 };

	renderPassInfo
		.setClearValueCount(static_cast<uint32>(cv.size())) //
		.setPClearValues(cv.data());

	cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	{
		cmdBuffer->setViewport(0, { viewport });
		cmdBuffer->setScissor(0, { scissor });

		m_postprocCollection.Draw(*cmdBuffer, sceneDesc);

		UnlitPass::RecordCmd(cmdBuffer, sceneDesc);
	}
	cmdBuffer->endRenderPass();

	// TODO: preparation of data for next frame should not be performed here?
	m_gbuffer->TransitionForWrite(cmdBuffer);

	for (auto sl : sceneDesc->spotlights.elements) {
		if (sl && sl->shadowmap) {
			sl->shadowmap->attachment->TransitionForWrite(cmdBuffer);
		}
	}

	for (auto dl : sceneDesc->directionalLights.elements) {
		if (dl && dl->shadowmap) {
			dl->shadowmap->attachment->TransitionForWrite(cmdBuffer);
		}
	}
}

void Renderer_::RecordOutPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc, vk::RenderPass outRp,
	vk::Framebuffer outFb, vk::Extent2D outExtent)
{
	PROFILE_SCOPE(Renderer);

	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(outRp) //
		.setFramebuffer(outFb);
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(outExtent);

	vk::ClearValue clearValue{};
	clearValue.setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	renderPassInfo
		.setClearValueCount(1u) //
		.setPClearValues(&clearValue);

	cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	{
		auto& scissor = m_viewportRect;
		const float x = static_cast<float>(scissor.offset.x);
		const float y = static_cast<float>(scissor.offset.y);
		const float width = static_cast<float>(scissor.extent.width);
		const float height = static_cast<float>(scissor.extent.height);

		vk::Viewport viewport{};
		viewport
			.setX(x) //
			.setY(y)
			.setWidth(width)
			.setHeight(height)
			.setMinDepth(0.f)
			.setMaxDepth(1.f);

		cmdBuffer->setViewport(0, { viewport });
		cmdBuffer->setScissor(0, { scissor });

		m_copyHdrTexture.RecordCmd(cmdBuffer);

		ImguiImpl::RenderVulkan(cmdBuffer);
	}
	cmdBuffer->endRenderPass();

	// transition for write again
	m_attachments[sceneDesc.frameIndex]->TransitionForWrite(cmdBuffer);
	m_attachments2[sceneDesc.frameIndex]->TransitionForWrite(cmdBuffer);
}

void Renderer_::OnViewportResize()
{
	vk::Extent2D viewportSize{ g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y };

	m_viewportRect.extent = viewportSize;
	m_viewportRect.offset = vk::Offset2D(g_ViewportCoordinates.position.x, g_ViewportCoordinates.position.y);

	vk::Extent2D fbSize = SuggestFramebufferSize(viewportSize);

	if (fbSize != m_viewportFramebufferSize) {
		m_viewportFramebufferSize = fbSize;

		m_gbuffer = std::make_unique<RGbuffer>(fbSize.width, fbSize.height);

		for (uint32 i = 0; i < c_framesInFlight; ++i) {
			m_attachments[i] = std::make_unique<RImageAttachment>("rgba32", fbSize.width, fbSize.height,
				vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
					| vk::ImageUsageFlagBits::eInputAttachment,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			m_attachments[i]->BlockingTransitionToLayout(
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

			m_attachments2[i] = std::make_unique<RImageAttachment>("rgba32", fbSize.width, fbSize.height,
				vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			m_attachments2[i]->BlockingTransitionToLayout(
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

			// descSets
			auto quadSampler = GpuAssetManager->GetDefaultSampler();

			vk::DescriptorImageInfo imageInfo{};
			imageInfo
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
				.setImageView(m_attachments2[i]->GetView())
				.setSampler(quadSampler);

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite
				.setDstSet(m_ppDescSets[i]) //
				.setDstBinding(0u)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1u)
				.setPBufferInfo(nullptr)
				.setPImageInfo(&imageInfo)
				.setPTexelBufferView(nullptr);

			Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);


			ptDebugObj->descSet[i] = ptDebugObj->descLayout.GetDescriptorSet();


			vk::DescriptorImageInfo imageInfo2{};
			imageInfo2
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
				.setImageView(Renderer->m_attachments[i]->GetView());
			//	.setSampler(VK_NULL_HANDLE);

			vk::WriteDescriptorSet descriptorWrite2{};
			descriptorWrite2
				.setDstSet(ptDebugObj->descSet[i]) //
				.setDstBinding(0)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eInputAttachment)
				.setDescriptorCount(1u)
				.setPBufferInfo(nullptr)
				.setPImageInfo(&imageInfo2)
				.setPTexelBufferView(nullptr);

			Device->updateDescriptorSets(1u, &descriptorWrite2, 0u, nullptr);


			std::array attch{ m_attachments[i]->GetView(), m_attachments2[i]->GetView(),
				m_gbuffer->attachments[GDepth]->GetView() };

			// framebuffer
			vk::FramebufferCreateInfo createInfo{};
			createInfo
				.setRenderPass(m_ptRenderpass.get()) //
				.setAttachmentCount(static_cast<uint32>(attch.size()))
				.setPAttachments(attch.data())
				.setWidth(fbSize.width)
				.setHeight(fbSize.height)
				.setLayers(1u);

			m_framebuffers[i] = Device->createFramebufferUnique(createInfo);
		}
	}
} // namespace vl

void Renderer_::UpdateForFrame()
{
	if (*m_didViewportResize) {
		OnViewportResize();
	}
}

void Renderer_::DrawFrame(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc, vk::RenderPass outRp,
	vk::Framebuffer outFb, vk::Extent2D outExtent)
{
	PROFILE_SCOPE(Renderer);

	m_secondaryBuffersPool.Top();

	// passes
	RecordGeometryPasses(cmdBuffer, sceneDesc);
	RecordPostProcessPass(cmdBuffer, sceneDesc);
	RecordOutPass(cmdBuffer, sceneDesc, outRp, outFb, outExtent);
}

} // namespace vl
