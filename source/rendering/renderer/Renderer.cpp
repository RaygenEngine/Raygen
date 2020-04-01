#include "pch.h"
#include "Renderer.h"

#include "editor/imgui/ImguiImpl.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/asset/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/wrapper/Swapchain.h"
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

	// create swapchain
	ReconstructSwapchain();
}

void Renderer_::Init()
{
	// Camera ubo WIP: (potentially used by many pipelines)
	m_cameraDescLayout.AddBinding(
		vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex /*| vk::ShaderStageFlagBits::eFragment*/);
	m_cameraDescLayout.Generate();

	for (int32 i = 0; i < swapchain->images.size(); ++i) {
		camDescSet.push_back(m_cameraDescLayout.GetDescriptorSet());
		cameraUBO.emplace_back(std::make_unique<Buffer>(sizeof(UBO_Material), vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
	}


	// CHECK: Code smell, needs internal first init function
	geomPass.InitAll();

	InitDebugDescriptors();

	defPass.InitQuadDescriptor();
	defPass.InitPipeline(swapchain->renderPass.get());


	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(Device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(static_cast<uint32>(swapchain->images.size()));

	geometryCmdBuffer = Device->allocateCommandBuffers(allocInfo);
	outCmdBuffer = Device->allocateCommandBuffers(allocInfo);


	vk::FenceCreateInfo fci{};
	fci.setFlags(vk::FenceCreateFlagBits::eSignaled);

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		renderFinishedSem.push_back(Device->createSemaphoreUnique({}));
		imageAvailSem.push_back(Device->createSemaphoreUnique({}));

		inFlightFence.push_back(Device->createFenceUnique(fci));
	}

	Event::OnViewportUpdated.BindFlag(this, didViewportResize);
	Event::OnWindowResize.BindFlag(this, didWindowResize);
	Event::OnWindowMinimize.Bind(this, [&](bool newIsMinimzed) { isMinimzed = newIsMinimzed; });
}

Renderer_::~Renderer_()
{
	delete swapchain;

	delete Scene;
}

void Renderer_::ReconstructSwapchain()
{
	delete swapchain;
	swapchain = new Swapchain(Instance->surface);
}

void Renderer_::InitDebugDescriptors()
{
	debugDescSetLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	debugDescSetLayout.Generate();
}


void Renderer_::OnViewportResize()
{
	vk::Extent2D viewportSize{ g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y };

	viewportRect.extent = viewportSize;
	viewportRect.offset = vk::Offset2D(g_ViewportCoordinates.position.x, g_ViewportCoordinates.position.y);

	vk::Extent2D fbSize = SuggestFramebufferSize(viewportSize);

	if (fbSize != viewportFramebufferSize) {
		viewportFramebufferSize = fbSize;
		geomPass.InitFramebuffers();
		defPass.UpdateDescriptorSets(*geomPass.m_gBuffer.get());
	}
}

void Renderer_::OnWindowResize()
{
	Device->waitIdle();
	ReconstructSwapchain();
}

void Renderer_::UpdateForFrame()
{
	if (*didWindowResize) {
		OnWindowResize();
	}

	if (*didViewportResize) {
		OnViewportResize();
	}

	Scene->ConsumeCmdQueue();
}

void Renderer_::DrawGeometryPass(vk::CommandBuffer cmdBuffer)
{

	geomPass.RecordGeometryDraw(&cmdBuffer);
}


void Renderer_::DrawDeferredPass(vk::CommandBuffer cmdBuffer, vk::Framebuffer framebuffer)
{
	PROFILE_SCOPE(Renderer);

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

	// begin command buffer recording
	cmdBuffer.begin(beginInfo);


	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo.setRenderPass(swapchain->renderPass.get()).setFramebuffer(framebuffer);
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(swapchain->extent);

	std::array<vk::ClearValue, 2> clearValues = {};
	clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[1].setDepthStencil({ 1.0f, 0 });
	renderPassInfo.setClearValueCount(static_cast<uint32>(clearValues.size()));
	renderPassInfo.setPClearValues(clearValues.data());

	cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	defPass.RecordCmd(&cmdBuffer);
	editorPass.RecordCmd(&cmdBuffer);

	cmdBuffer.endRenderPass();
	geomPass.m_gBuffer->TransitionForAttachmentWrite(cmdBuffer);

	cmdBuffer.end();
}

void Renderer_::UpdateCamera()
{
	// update camera UBO
	auto camera = Scene->GetActiveCamera();
	if (camera) {

		UBO_Camera camData{ camera->viewProj };

		cameraUBO[currentFrame]->UploadData(&camData, sizeof(UBO_Camera));

		vk::DescriptorBufferInfo bufferInfo{};

		bufferInfo
			.setBuffer(*cameraUBO[currentFrame]) //
			.setOffset(0u)
			.setRange(sizeof(UBO_Camera));
		vk::WriteDescriptorSet descriptorWrite{};

		descriptorWrite
			.setDstSet(camDescSet[currentFrame]) //
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
	if (isMinimzed) {
		return;
	}

	UpdateForFrame();

	PROFILE_SCOPE(Renderer);
	uint32 imageIndex;

	currentFrame = (currentFrame + 1) % c_framesInFlight;

	Device->waitForFences({ *inFlightFence[currentFrame] }, true, UINT64_MAX);
	Device->resetFences({ *inFlightFence[currentFrame] });

	UpdateCamera();

	Device->acquireNextImageKHR(swapchain->handle.get(), UINT64_MAX, { *imageAvailSem[currentFrame] }, {}, &imageIndex);

	DrawGeometryPass(geometryCmdBuffer[currentFrame]);
	DrawDeferredPass(outCmdBuffer[currentFrame], swapchain->framebuffers[currentFrame].get());

	std::array bufs = { geometryCmdBuffer[currentFrame], outCmdBuffer[currentFrame] };


	std::array<vk::PipelineStageFlags, 1> waitStage = { vk::PipelineStageFlagBits::eColorAttachmentOutput }; //
	std::array waitSems = { *imageAvailSem[currentFrame] };                                                  //
	std::array signalSems = { *renderFinishedSem[currentFrame] };


	vk::SubmitInfo submitInfo{};
	submitInfo
		.setWaitSemaphoreCount(static_cast<uint32>(waitSems.size())) //
		.setPWaitSemaphores(waitSems.data())
		.setPWaitDstStageMask(waitStage.data())

		.setSignalSemaphoreCount(static_cast<uint32>(signalSems.size()))
		.setPSignalSemaphores(signalSems.data())

		.setCommandBufferCount(2u)
		.setPCommandBuffers(bufs.data());


	Device->graphicsQueue.submit(1u, &submitInfo, *inFlightFence[currentFrame]);


	vk::PresentInfoKHR presentInfo;
	presentInfo //
		.setWaitSemaphoreCount(1u)
		.setPWaitSemaphores(&*renderFinishedSem[currentFrame]);


	vk::SwapchainKHR swapChains[] = { swapchain->handle.get() };
	presentInfo //
		.setSwapchainCount(1u)
		.setPSwapchains(swapChains)
		.setPImageIndices(&imageIndex)
		.setPResults(nullptr);


	PROFILE_SCOPE(Renderer);
	Device->presentQueue.presentKHR(presentInfo);
}
} // namespace vl
