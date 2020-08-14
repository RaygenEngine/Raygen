#include "pch.h"
#include "Layer.h"

#include "engine/console/ConsoleVariable.h"
#include "platform/Platform.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/scene/Scene.h"
#include "rendering/VulkanLoader.h"
#include "rendering/wrappers/Swapchain.h"
#include "engine/Events.h"

#include "universe/Universe.h"
#include "engine/Input.h"
#include "universe/systems/SceneCmdSystem.h"
#include "editor/EditorObject.h"
#include "rendering/scene/SceneSpotlight.h"
#include "engine/profiler/ProfileScope.h"

ConsoleFunction<> console_BuildAll{ "s.buildAll", []() { vl::Layer->mainScene->BuildAll(); },
	"Builds all build-able scene nodes" };
ConsoleFunction<> console_BuildAS{ "s.buildTestAccelerationStructure", []() {},
	"Builds a top level acceleration structure, for debugging purposes, todo: remove" };

namespace vl {


Layer_::Layer_()
{
	VulkanLoader::InitLoaderBase();

	Instance = new Instance_(Platform::GetVulkanExtensions(), Platform::GetMainHandle());

	Device = new Device_(Instance->physicalDevices[0]);

	GpuResources = new GpuResources_();
	GpuAssetManager = new GpuAssetManager_();

	Layouts = new Layouts_();

	mainSwapchain = new RSwapchain(Instance->surface);

	mainScene = new Scene();
	currentScene = mainScene;

	Renderer = new Renderer_();
	Renderer->InitPipelines(mainSwapchain->renderPass.get());

	vk::FenceCreateInfo fci{};
	fci.setFlags(vk::FenceCreateFlagBits::eSignaled);

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		m_renderFinishedSem[i] = Device->createSemaphoreUnique({});
		m_imageAvailSem[i] = Device->createSemaphoreUnique({});
		m_inFlightFence[i] = Device->createFenceUnique(fci);
	}


	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(Device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(c_framesInFlight);

	m_cmdBuffer = Device->allocateCommandBuffers(allocInfo);

	Event::OnWindowResize.BindFlag(this, m_didWindowResize);
	Event::OnWindowMinimize.Bind(this, [&](bool newIsMinimized) { m_isMinimized = newIsMinimized; });


} // namespace vl

Layer_::~Layer_()
{
	delete Renderer;

	// WIP:
	delete mainSwapchain;
	delete mainScene;

	delete Layouts;

	delete GpuAssetManager;
	delete GpuResources;

	m_inFlightFence = {};
	m_renderFinishedSem = {};
	m_imageAvailSem = {};

	delete Device;
	delete Instance;
}

void Layer_::DrawFrame()
{
	currentScene = mainScene;


	if (*m_didWindowResize) {
		Device->waitIdle();

		delete mainSwapchain;
		mainSwapchain = new RSwapchain(Instance->surface);
	}

	if (m_isMinimized)
		[[unlikely]] { return; }

	Renderer->PrepareForFrame();

	GpuAssetManager->ConsumeAssetUpdates();

	currentScene->ConsumeCmdQueue();

	currentFrame = (currentFrame + 1) % c_framesInFlight;
	auto currentCmdBuffer = &m_cmdBuffer[currentFrame];

	{
		PROFILE_SCOPE(Renderer);

		Device->waitForFences({ *m_inFlightFence[currentFrame] }, true, UINT64_MAX);
		Device->resetFences({ *m_inFlightFence[currentFrame] });

		currentScene->UploadDirty(currentFrame);
	}
	uint32 imageIndex;

	Device->acquireNextImageKHR(*mainSwapchain, UINT64_MAX, { m_imageAvailSem[currentFrame].get() }, {}, &imageIndex);


	auto outRp = mainSwapchain->renderPass.get();
	auto outFb = mainSwapchain->framebuffers[imageIndex].get();
	auto outExtent = mainSwapchain->extent;

	// WIP: 0 = editor camera
	SceneRenderDesc sceneDesc{ currentScene, 0, currentFrame };


	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo
		.setFlags(vk::CommandBufferUsageFlags(0)) //
		.setPInheritanceInfo(nullptr);

	currentCmdBuffer->begin(beginInfo);
	{
		Renderer->DrawFrame(currentCmdBuffer, sceneDesc, outRp, outFb, outExtent);
	}
	currentCmdBuffer->end();


	std::array<vk::PipelineStageFlags, 1> waitStage = { vk::PipelineStageFlagBits::eColorAttachmentOutput }; //
	std::array waitSems = { *m_imageAvailSem[currentFrame] };                                                //
	std::array signalSems = { *m_renderFinishedSem[currentFrame] };

	std::array bufs = { m_cmdBuffer[currentFrame] };

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

	vk::SwapchainKHR swapChains[] = { *mainSwapchain };

	vk::PresentInfoKHR presentInfo;
	presentInfo //
		.setWaitSemaphoreCount(1u)
		.setPWaitSemaphores(&m_renderFinishedSem[currentFrame].get())
		.setSwapchainCount(1u)
		.setPSwapchains(swapChains)
		.setPImageIndices(&imageIndex)
		.setPResults(nullptr);


	Device->presentQueue.presentKHR(presentInfo);
}
} // namespace vl
