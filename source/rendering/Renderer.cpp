#include "pch.h"
#include "Renderer.h"

#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/passes/GBufferPass.h"
#include "rendering/ppt/techniques/PtDebug.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneDirectionalLight.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/Swapchain.h"
#include "rendering/VulkanUtl.h"

constexpr int32 c_framesInFlight = 2;
namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {

Renderer_::Renderer_()
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(Device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(Swapchain->GetImageCount());

	m_geometryCmdBuffer = Device->allocateCommandBuffers(allocInfo);
	m_pptCmdBuffer = Device->allocateCommandBuffers(allocInfo);
	m_outCmdBuffer = Device->allocateCommandBuffers(allocInfo);

	vk::FenceCreateInfo fci{};
	fci.setFlags(vk::FenceCreateFlagBits::eSignaled);

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		m_renderFinishedSem.push_back(Device->createSemaphoreUnique({}));
		m_imageAvailSem.push_back(Device->createSemaphoreUnique({}));

		m_inFlightFence.push_back(Device->createFenceUnique(fci));
	}

	Event::OnViewportUpdated.BindFlag(this, m_didViewportResize);
	Event::OnWindowResize.BindFlag(this, m_didWindowResize);
	Event::OnWindowMinimize.Bind(this, [&](bool newIsMinimzed) { m_isMinimzed = newIsMinimzed; });

	// post process render pass

	vk::AttachmentDescription colorAttachmentDesc{};
	vk::AttachmentReference colorAttachmentRef{};

	colorAttachmentDesc.setFormat(vk::Format::eR32G32B32A32Sfloat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	colorAttachmentRef
		.setAttachment(0u) //
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);


	vk::AttachmentDescription colorAttachmentDesc2{};
	vk::AttachmentReference colorAttachmentRef2{};

	colorAttachmentDesc2.setFormat(vk::Format::eR32G32B32A32Sfloat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
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
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
		.setDependencyFlags(vk::DependencyFlagBits::eByRegion);


	vk::SubpassDescription debugSupass{};
	debugSupass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachmentCount(1u)
		.setInputAttachmentCount(1u)
		.setPInputAttachments(&colorAttachmentRef)
		.setPColorAttachments(&colorAttachmentRef2)
		.setPDepthStencilAttachment(nullptr);


	vk::SubpassDependency debugDep{};
	debugDep
		.setSrcSubpass(0u) //
		.setDstSubpass(1u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
		.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	std::array subpasses{ lightSubpass, debugSupass };
	std::array dependcies{ lightDep, debugDep };
	std::array attachments{ colorAttachmentDesc, colorAttachmentDesc2 };


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
	for (uint32 i = 0; i < 3; ++i) {
		m_ppDescSets[i] = Layouts->singleSamplerDescLayout.GetDescriptorSet();
	}
}

void Renderer_::InitPipelines()
{
	m_copyHdrTexture.MakePipeline();
	m_postprocCollection.RegisterTechniques();
}

Renderer_::~Renderer_() {}

void Renderer_::RecordGeometryPasses(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo
		.setFlags(vk::CommandBufferUsageFlags(0)) //
		.setPInheritanceInfo(nullptr);

	cmdBuffer->begin(beginInfo);
	{
		GbufferPass::RecordCmd(
			cmdBuffer, m_gbuffer.get(), Scene->geometries.elements, Scene->animatedGeometries.elements);

		for (auto sl : Scene->spotlights.elements) {
			if (sl) {
				DepthmapPass::RecordCmd(cmdBuffer, *sl->shadowmap, sl->ubo.viewProj, Scene->geometries.elements,
					Scene->animatedGeometries.elements);
			}
		}

		for (auto dl : Scene->directionalLights.elements) {
			if (dl) {
				DepthmapPass::RecordCmd(cmdBuffer, *dl->shadowmap, dl->ubo.viewProj, Scene->geometries.elements,
					Scene->animatedGeometries.elements);
			}
		}
	}
	cmdBuffer->end();
}

void Renderer_::RecordPostProcessPass(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);

	// TODO: each effect except the maybe the lightpasses should use its own render/buffer scale
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


	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo
		.setFlags(vk::CommandBufferUsageFlags(0)) //
		.setPInheritanceInfo(nullptr);

	cmdBuffer->begin(beginInfo);
	{
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo
			.setRenderPass(m_ptRenderpass.get()) //
			.setFramebuffer(m_framebuffers[currentFrame].get());
		renderPassInfo.renderArea
			.setOffset({ 0, 0 }) //
			.setExtent(extent);  // CHECK:

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
			// Dynamic viewport & scissor
			cmdBuffer->setViewport(0, { viewport });
			cmdBuffer->setScissor(0, { scissor });

			m_postprocCollection.Draw(*cmdBuffer, currentFrame);
		}
		cmdBuffer->endRenderPass();

		// TODO: preparation of data for next frame should not be performed here?
		m_gbuffer->TransitionForWrite(cmdBuffer);

		for (auto sl : Scene->spotlights.elements) {
			if (sl && sl->shadowmap) {
				sl->shadowmap->TransitionForWrite(cmdBuffer);
			}
		}

		for (auto dl : Scene->directionalLights.elements) {
			if (dl && dl->shadowmap) {
				dl->shadowmap->TransitionForWrite(cmdBuffer);
			}
		}
	}
	cmdBuffer->end();
}

void Renderer_::RecordOutPass(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo
		.setFlags(vk::CommandBufferUsageFlags(0)) //
		.setPInheritanceInfo(nullptr);

	cmdBuffer->begin(beginInfo);
	{
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo
			.setRenderPass(Swapchain->GetRenderPass()) //
			.setFramebuffer(Swapchain->GetFramebuffer(Renderer->currentFrame));
		renderPassInfo.renderArea
			.setOffset({ 0, 0 }) //
			.setExtent(Swapchain->GetExtent());

		vk::ClearValue clearValue{};
		clearValue.setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
		renderPassInfo
			.setClearValueCount(1u) //
			.setPClearValues(&clearValue);

		cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		{
			auto scissor = GetGameScissor();
			auto viewport = GetGameViewport();

			cmdBuffer->setViewport(0, { viewport });
			cmdBuffer->setScissor(0, { scissor });

			m_copyHdrTexture.RecordCmd(cmdBuffer);
			m_writeEditor.RecordCmd(cmdBuffer);
		}
		cmdBuffer->endRenderPass();

		// transition for write again (TODO: image function)
		auto barrier = m_attachments[currentFrame]->CreateTransitionBarrier(
			vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
		vk::PipelineStageFlags sourceStage = GetPipelineStage(vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::PipelineStageFlags destinationStage = GetPipelineStage(vk::ImageLayout::eColorAttachmentOptimal);
		cmdBuffer->pipelineBarrier(
			sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });

		auto barrier2 = m_attachments2[currentFrame]->CreateTransitionBarrier(
			vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
		vk::PipelineStageFlags sourceStage2 = GetPipelineStage(vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::PipelineStageFlags destinationStage2 = GetPipelineStage(vk::ImageLayout::eColorAttachmentOptimal);
		cmdBuffer->pipelineBarrier(
			sourceStage2, destinationStage2, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier2 });
	}
	cmdBuffer->end();
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

		for (uint32 i = 0; i < 3; ++i) {
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


			PtDebug::descSet[i] = PtDebug::descLayout.GetDescriptorSet();


			vk::DescriptorImageInfo imageInfo2{};
			imageInfo2
				.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal) //
				.setImageView(Renderer->m_attachments[i]->GetView());
			//	.setSampler(VK_NULL_HANDLE);

			vk::WriteDescriptorSet descriptorWrite2{};
			descriptorWrite2
				.setDstSet(PtDebug::descSet[i]) //
				.setDstBinding(0)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eInputAttachment)
				.setDescriptorCount(1u)
				.setPBufferInfo(nullptr)
				.setPImageInfo(&imageInfo2)
				.setPTexelBufferView(nullptr);

			Device->updateDescriptorSets(1u, &descriptorWrite2, 0u, nullptr);


			std::array attch{ m_attachments[i]->GetView(), m_attachments2[i]->GetView() };
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

void Renderer_::OnWindowResize()
{
	Device->waitIdle();

	delete Swapchain;
	Swapchain = new Swapchain_(Instance->surface);
}

void Renderer_::UpdateForFrame()
{
	if (*m_didWindowResize) {
		OnWindowResize();
	}

	if (*m_didViewportResize) {
		OnViewportResize();
	}

	GpuAssetManager->ConsumeAssetUpdates();
	Scene->ConsumeCmdQueue();
}

void Renderer_::DrawFrame()
{
	if (m_isMinimzed) {
		return;
	}

	UpdateForFrame();

	PROFILE_SCOPE(Renderer);
	uint32 imageIndex;

	currentFrame = (currentFrame + 1) % c_framesInFlight;

	Device->waitForFences({ *m_inFlightFence[currentFrame] }, true, UINT64_MAX);
	Device->resetFences({ *m_inFlightFence[currentFrame] });

	Scene->UploadDirty();

	Device->acquireNextImageKHR(*Swapchain, UINT64_MAX, { *m_imageAvailSem[currentFrame] }, {}, &imageIndex);

	// passes
	RecordGeometryPasses(&m_geometryCmdBuffer[currentFrame]);
	RecordPostProcessPass(&m_pptCmdBuffer[currentFrame]);
	RecordOutPass(&m_outCmdBuffer[currentFrame]);

	std::array bufs = { m_geometryCmdBuffer[currentFrame], m_pptCmdBuffer[currentFrame], m_outCmdBuffer[currentFrame] };

	std::array<vk::PipelineStageFlags, 1> waitStage = { vk::PipelineStageFlagBits::eColorAttachmentOutput }; //
	std::array waitSems = { *m_imageAvailSem[currentFrame] };                                                //
	std::array signalSems = { *m_renderFinishedSem[currentFrame] };


	vk::SubmitInfo submitInfo{};
	submitInfo
		.setWaitSemaphoreCount(static_cast<uint32>(waitSems.size())) //
		.setPWaitSemaphores(waitSems.data())
		.setPWaitDstStageMask(waitStage.data())

		.setSignalSemaphoreCount(static_cast<uint32>(signalSems.size()))
		.setPSignalSemaphores(signalSems.data())

		.setCommandBufferCount(static_cast<uint32>(bufs.size()))
		.setPCommandBuffers(bufs.data());


	Device->graphicsQueue.submit(1u, &submitInfo, *m_inFlightFence[currentFrame]);


	vk::PresentInfoKHR presentInfo;
	presentInfo //
		.setWaitSemaphoreCount(1u)
		.setPWaitSemaphores(&*m_renderFinishedSem[currentFrame]);


	vk::SwapchainKHR swapChains[] = { *Swapchain };
	presentInfo //
		.setSwapchainCount(1u)
		.setPSwapchains(swapChains)
		.setPImageIndices(&imageIndex)
		.setPResults(nullptr);


	PROFILE_SCOPE(Renderer);
	Device->presentQueue.presentKHR(presentInfo);
}

vk::Viewport Renderer_::GetSceneViewport() const
{
	auto vpSize = m_viewportRect.extent;

	vk::Viewport viewport{};
	viewport
		.setX(0) //
		.setY(0)
		.setWidth(static_cast<float>(vpSize.width))
		.setHeight(static_cast<float>(vpSize.height))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);
	return viewport;
}

vk::Viewport Renderer_::GetGameViewport() const
{
	auto& rect = m_viewportRect;
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

vk::Rect2D Renderer_::GetSceneScissor() const
{
	vk::Rect2D scissor{};

	scissor
		.setOffset({ 0, 0 }) //
		.setExtent(m_viewportRect.extent);

	return scissor;
}

vk::Rect2D Renderer_::GetGameScissor() const
{
	return m_viewportRect;
}
} // namespace vl
