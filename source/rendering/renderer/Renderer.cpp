#include "pch.h"
#include "Renderer.h"

#include "editor/imgui/ImguiImpl.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
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
	Scene = new Scene_();

	// create m_swapchain
	ReconstructSwapchain();
}

void Renderer_::Init()
{
	// Camera ubo WIP: (potentially used by many pipelines)
	m_cameraDescLayout.AddBinding(
		vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex /*| vk::ShaderStageFlagBits::eFragment*/);
	m_cameraDescLayout.Generate();

	for (int32 i = 0; i < m_swapchain->images.size(); ++i) {
		m_camDescSet.push_back(m_cameraDescLayout.GetDescriptorSet());
		m_cameraUBO.emplace_back(std::make_unique<Buffer<UBO_Camera>>(vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
	}


	// CHECK: Code smell, needs internal first init function
	m_geomPass.InitAll();

	m_defPass.InitQuadDescriptor();
	m_defPass.InitPipeline(m_swapchain->renderPass.get());


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
	delete m_swapchain;

	delete Scene;
}

void Renderer_::ReconstructSwapchain()
{
	delete m_swapchain;
	m_swapchain = new Swapchain(Instance->surface);
}

void Renderer_::OnViewportResize()
{
	vk::Extent2D viewportSize{ g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y };

	m_viewportRect.extent = viewportSize;
	m_viewportRect.offset = vk::Offset2D(g_ViewportCoordinates.position.x, g_ViewportCoordinates.position.y);

	vk::Extent2D fbSize = SuggestFramebufferSize(viewportSize);

	if (fbSize != m_viewportFramebufferSize) {
		m_viewportFramebufferSize = fbSize;
		m_geomPass.InitFramebuffers();
		m_defPass.UpdateDescriptorSets(*m_geomPass.GetGBuffer());
	}
}

void Renderer_::OnWindowResize()
{
	Device->waitIdle();
	ReconstructSwapchain();
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

	m_geomPass.RecordGeometryDraw(&cmdBuffer);
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

	m_defPass.RecordCmd(&cmdBuffer);
	m_editorPass.RecordCmd(&cmdBuffer);

	cmdBuffer.endRenderPass();
	m_geomPass.GetGBuffer()->TransitionForAttachmentWrite(cmdBuffer);

	cmdBuffer.end();
}

void Renderer_::UpdateCamera()
{
	// update camera UBO
	auto camera = Scene->GetActiveCamera();
	if (camera) {

		UBO_Camera camData{ camera->viewProj };

		m_cameraUBO[m_currentFrame]->UploadData(camData);

		vk::DescriptorBufferInfo bufferInfo{};

		bufferInfo
			.setBuffer(*m_cameraUBO[m_currentFrame]) //
			.setOffset(0u)
			.setRange(sizeof(UBO_Camera));
		vk::WriteDescriptorSet descriptorWrite{};

		descriptorWrite
			.setDstSet(m_camDescSet[m_currentFrame]) //
			.setDstBinding(0u)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1u)
			.setPBufferInfo(&bufferInfo)
			.setPImageInfo(nullptr)
			.setPTexelBufferView(nullptr);

		Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
}

void Renderer_::DrawFrame()
{
	if (m_isMinimzed) {
		return;
	}

	UpdateForFrame();

	PROFILE_SCOPE(Renderer);
	uint32 imageIndex;

	m_currentFrame = (m_currentFrame + 1) % c_framesInFlight;

	Device->waitForFences({ *m_inFlightFence[m_currentFrame] }, true, UINT64_MAX);
	Device->resetFences({ *m_inFlightFence[m_currentFrame] });

	UpdateCamera();

	Device->acquireNextImageKHR(
		m_swapchain->handle.get(), UINT64_MAX, { *m_imageAvailSem[m_currentFrame] }, {}, &imageIndex);

	DrawGeometryPass(m_geometryCmdBuffer[m_currentFrame]);
	DrawDeferredPass(m_outCmdBuffer[m_currentFrame], m_swapchain->framebuffers[m_currentFrame].get());

	std::array bufs = { m_geometryCmdBuffer[m_currentFrame], m_outCmdBuffer[m_currentFrame] };


	std::array<vk::PipelineStageFlags, 1> waitStage = { vk::PipelineStageFlagBits::eColorAttachmentOutput }; //
	std::array waitSems = { *m_imageAvailSem[m_currentFrame] };                                              //
	std::array signalSems = { *m_renderFinishedSem[m_currentFrame] };


	vk::SubmitInfo submitInfo{};
	submitInfo
		.setWaitSemaphoreCount(static_cast<uint32>(waitSems.size())) //
		.setPWaitSemaphores(waitSems.data())
		.setPWaitDstStageMask(waitStage.data())

		.setSignalSemaphoreCount(static_cast<uint32>(signalSems.size()))
		.setPSignalSemaphores(signalSems.data())

		.setCommandBufferCount(2u)
		.setPCommandBuffers(bufs.data());


	Device->graphicsQueue.submit(1u, &submitInfo, *m_inFlightFence[m_currentFrame]);


	vk::PresentInfoKHR presentInfo;
	presentInfo //
		.setWaitSemaphoreCount(1u)
		.setPWaitSemaphores(&*m_renderFinishedSem[m_currentFrame]);


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
