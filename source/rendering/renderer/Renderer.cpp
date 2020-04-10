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
#include "universe/World.h"

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
	m_swapchain = std::make_unique<Swapchain>(Instance->surface);

	Scene = new Scene_(m_swapchain->images.size());

	m_geomPass = std::make_unique<GeometryPass>();
	m_defPass = std::make_unique<DeferredPass>(m_swapchain->renderPass.get());
	m_editorPass = std::make_unique<EditorPass>();

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(Device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(static_cast<uint32>(m_swapchain->images.size()));

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
}

Renderer_::~Renderer_()
{
	delete Scene;
}

void Renderer_::OnViewportResize()
{
	vk::Extent2D viewportSize{ g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y };

	m_viewportRect.extent = viewportSize;
	m_viewportRect.offset = vk::Offset2D(g_ViewportCoordinates.position.x, g_ViewportCoordinates.position.y);

	vk::Extent2D fbSize = SuggestFramebufferSize(viewportSize);

	if (fbSize != m_viewportFramebufferSize) {
		m_viewportFramebufferSize = fbSize;
		m_gBuffer = std::make_unique<GBuffer>(fbSize.width, fbSize.height);
		m_geomPass->MakeFramebuffers(*m_gBuffer);
		m_defPass->UpdateDescriptorSet(*m_gBuffer);
	}
}

void Renderer_::OnWindowResize()
{
	Device->waitIdle();
	m_swapchain = std::make_unique<Swapchain>(Instance->surface);
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

void Renderer_::DrawGeometryPass(vk::CommandBuffer cmdBuffer)
{
	m_geomPass->RecordGeometryDraw(&cmdBuffer);
}


void Renderer_::DrawDeferredPass(vk::CommandBuffer cmdBuffer, vk::Framebuffer framebuffer)
{
	PROFILE_SCOPE(Renderer);

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

	// begin command buffer recording
	cmdBuffer.begin(beginInfo);


	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo.setRenderPass(m_swapchain->renderPass.get()).setFramebuffer(framebuffer);
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(m_swapchain->extent);

	std::array<vk::ClearValue, 2> clearValues = {};
	clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[1].setDepthStencil({ 1.0f, 0 });
	renderPassInfo.setClearValueCount(static_cast<uint32>(clearValues.size()));
	renderPassInfo.setPClearValues(clearValues.data());

	cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	m_defPass->RecordCmd(&cmdBuffer);
	m_editorPass->RecordCmd(&cmdBuffer);

	cmdBuffer.endRenderPass();

	m_gBuffer->TransitionForAttachmentWrite(cmdBuffer);

	cmdBuffer.end();
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

	Device->acquireNextImageKHR(
		m_swapchain->handle.get(), UINT64_MAX, { *m_imageAvailSem[currentFrame] }, {}, &imageIndex);

	DrawGeometryPass(m_geometryCmdBuffer[currentFrame]);
	DrawDeferredPass(m_outCmdBuffer[currentFrame], m_swapchain->framebuffers[currentFrame].get());

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

		.setCommandBufferCount(2u)
		.setPCommandBuffers(bufs.data());


	Device->graphicsQueue.submit(1u, &submitInfo, *m_inFlightFence[currentFrame]);


	vk::PresentInfoKHR presentInfo;
	presentInfo //
		.setWaitSemaphoreCount(1u)
		.setPWaitSemaphores(&*m_renderFinishedSem[currentFrame]);


	vk::SwapchainKHR swapChains[] = { m_swapchain->handle.get() };
	presentInfo //
		.setSwapchainCount(1u)
		.setPSwapchains(swapChains)
		.setPImageIndices(&imageIndex)
		.setPResults(nullptr);


	PROFILE_SCOPE(Renderer);
	Device->presentQueue.presentKHR(presentInfo);
}
} // namespace vl
