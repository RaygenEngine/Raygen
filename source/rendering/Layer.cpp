#include "pch.h"
#include "Layer.h"

#include "assets/GpuAssetManager.h"
#include "Device.h"
#include "editor/EditorObject.h"
#include "engine/console/ConsoleVariable.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
#include "rendering/Instance.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
#include "rendering/VulkanLoader.h"
#include "resource/GpuResources.h"
#include "scene/Scene.h"
#include "scene/SceneSpotlight.h"
#include "universe/systems/SceneCmdSystem.h"
#include "universe/Universe.h"
#include "wrappers/Swapchain.h"

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

	GpuResources::Init();
	GpuAssetManager = new GpuAssetManager_();

	Layouts = new Layouts_();

	mainSwapchain = new RSwapchain(Instance->surface);

	mainScene = new Scene();
	currentScene = mainScene;

	Renderer = new Renderer_();
	Renderer->InitPipelines(mainSwapchain->renderPass.get());

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		m_renderFinishedSems[i] = Device->createSemaphoreUnique({});
		m_imageAvailSems[i] = Device->createSemaphoreUnique({});
		m_fences[i] = Device->createFenceUnique({ vk::FenceCreateFlagBits::eSignaled });

		DEBUG_NAME(m_renderFinishedSems[i], "Renderer Finished" + std::to_string(i));
		DEBUG_NAME(m_imageAvailSems[i], "Image Available" + std::to_string(i));
	}

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(Device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(c_framesInFlight);

	m_cmdBuffers = Device->allocateCommandBuffers(allocInfo);

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
	GpuResources::Destroy();

	m_fences = {};
	m_renderFinishedSems = {};
	m_imageAvailSems = {};

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
	auto currentCmdBuffer = &m_cmdBuffers[currentFrame];

	{
		PROFILE_SCOPE(Renderer);

		Device->waitForFences({ *m_fences[currentFrame] }, true, UINT64_MAX);
		Device->resetFences({ *m_fences[currentFrame] });

		currentScene->UploadDirty(currentFrame);
	}
	uint32 imageIndex;

	Device->acquireNextImageKHR(*mainSwapchain, UINT64_MAX, { m_imageAvailSems[currentFrame].get() }, {}, &imageIndex);


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
	std::array waitSems = { *m_imageAvailSems[currentFrame] };                                                //
	std::array signalSems = { *m_renderFinishedSems[currentFrame] };

	std::array bufs = { m_cmdBuffers[currentFrame] };

	vk::SubmitInfo submitInfo{};
	submitInfo
		.setWaitSemaphoreCount(static_cast<uint32>(waitSems.size())) //
		.setPWaitSemaphores(waitSems.data())
		.setPWaitDstStageMask(waitStage.data())

		.setSignalSemaphoreCount(static_cast<uint32>(signalSems.size()))
		.setPSignalSemaphores(signalSems.data())

		.setCommandBufferCount(static_cast<uint32>(bufs.size()))
		.setPCommandBuffers(bufs.data());

	Device->graphicsQueue.submit(1u, &submitInfo, *m_fences[currentFrame]);


	vk::SwapchainKHR swapChains[] = { *mainSwapchain };

	vk::PresentInfoKHR presentInfo;
	presentInfo //
		.setWaitSemaphoreCount(1u)
		.setPWaitSemaphores(&m_renderFinishedSems[currentFrame].get())
		.setSwapchainCount(1u)
		.setPSwapchains(swapChains)
		.setPImageIndices(&imageIndex)
		.setPResults(nullptr);


	Device->presentQueue.presentKHR(presentInfo);
}
} // namespace vl
