#include "pch.h"
#include "Renderer.h"

#include "editor/imgui/ImguiImpl.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "engine/Logger.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "universe/nodes/camera/CameraNode.h"
#include "universe/nodes/geometry/GeometryNode.h"

#include <array>

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

	m_gBufferPass.MakePipeline();
	m_shadowmapPass.MakePipeline();
	m_spotlightPass.MakePipeline();
	m_ambientPass.MakePipeline();
}

Renderer_::~Renderer_() {}

void Renderer_::RecordGeometryPasses(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);
	auto viewport = Renderer->GetSceneViewport();
	auto scissor = Renderer->GetSceneScissor();

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

	// begin command buffer recording
	cmdBuffer->begin(beginInfo);
	{
		m_gBufferPass.RecordCmd(cmdBuffer, viewport, scissor);
		m_shadowmapPass.RecordCmd(cmdBuffer);
	}
	// end command buffer recording
	cmdBuffer->end();
}

void Renderer_::RecordDeferredPasses(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);
	auto viewport = Renderer->GetViewport();
	auto scissor = Renderer->GetScissor();

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

	// begin command buffer recording
	cmdBuffer->begin(beginInfo);
	{
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo
			.setRenderPass(Swapchain->GetRenderPass()) //
			.setFramebuffer(Swapchain->GetFramebuffer(Renderer->currentFrame));
		renderPassInfo.renderArea
			.setOffset({ 0, 0 }) //
			.setExtent(Swapchain->GetExtent());

		std::array<vk::ClearValue, 2> clearValues = {};
		clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
		clearValues[1].setDepthStencil({ 1.0f, 0 });
		renderPassInfo.setClearValueCount(static_cast<uint32>(clearValues.size()));
		renderPassInfo.setPClearValues(clearValues.data());

		cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		{
			m_spotlightPass.RecordCmd(cmdBuffer, viewport, scissor);
			// all lights
			m_ambientPass.RecordCmd(cmdBuffer, viewport, scissor);
			// post process
			m_editorPass.RecordCmd(cmdBuffer);
		}
		cmdBuffer->endRenderPass();

		m_gBuffer->TransitionForWrite(cmdBuffer);

		for (auto sl : Scene->spotlights.elements) {
			if (sl && sl->shadowmap) {
				sl->shadowmap->TransitionForWrite(cmdBuffer);
			}
		}
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
		m_gBuffer = std::make_unique<GBuffer>(m_gBufferPass.GetRenderPass(), fbSize.width, fbSize.height);
	}
}

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
	RecordDeferredPasses(&m_outCmdBuffer[currentFrame]);

	std::array bufs = { m_geometryCmdBuffer[currentFrame], m_outCmdBuffer[currentFrame] };

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

vk::Viewport Renderer_::GetViewport() const
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

vk::Rect2D Renderer_::GetScissor() const
{
	return m_viewportRect;
}
} // namespace vl
