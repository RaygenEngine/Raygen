#include "pch.h"
#include "Layer.h"

#include "assets/GpuAssetManager.h"
#include "editor/EditorObject.h"
#include "engine/console/ConsoleVariable.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "platform/Platform.h"
#include "rendering/Device.h"
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
	Renderer->InitPipelines(mainSwapchain->renderPass());

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		m_renderFinishedSem[i] = Device->createSemaphoreUnique({});
		m_imageAvailSem[i] = Device->createSemaphoreUnique({});
		m_frameFence[i] = Device->createFenceUnique({ vk::FenceCreateFlagBits::eSignaled });

		DEBUG_NAME(m_renderFinishedSem[i], "Renderer Finished" + std::to_string(i));
		DEBUG_NAME(m_imageAvailSem[i], "Image Available" + std::to_string(i));
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
	GpuResources::Destroy();

	m_frameFence = {};
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

	m_currentFrame = (m_currentFrame + 1) % c_framesInFlight;
	auto currentCmdBuffer = &m_cmdBuffer[m_currentFrame];

	{
		PROFILE_SCOPE(Renderer);

		Device->waitForFences({ *m_frameFence[m_currentFrame] }, true, UINT64_MAX);
		Device->resetFences({ *m_frameFence[m_currentFrame] });

		currentScene->UploadDirty(m_currentFrame);
	}

	uint32 imageIndex;
	Device->acquireNextImageKHR(
		mainSwapchain->handle(), UINT64_MAX, { m_imageAvailSem[m_currentFrame].get() }, {}, &imageIndex);


	auto outRp = mainSwapchain->renderPass();
	auto outFb = mainSwapchain->framebuffer(imageIndex);
	auto outExtent = mainSwapchain->extent;

	// WIP: 0 = editor camera
	SceneRenderDesc sceneDesc{ currentScene, 0, m_currentFrame };

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo
		.setFlags(vk::CommandBufferUsageFlags(0)) //
		.setPInheritanceInfo(nullptr);

	currentCmdBuffer->begin(beginInfo);
	{
		Renderer->DrawFrame(currentCmdBuffer, sceneDesc, outRp, outFb, outExtent);
	}
	currentCmdBuffer->end();


	std::array<vk::PipelineStageFlags, 1> waitStage = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	std::array waitSems = { *m_imageAvailSem[m_currentFrame] };
	std::array signalSems = { *m_renderFinishedSem[m_currentFrame] };

	std::array bufs = { m_cmdBuffer[m_currentFrame] };

	vk::SubmitInfo submitInfo{};
	submitInfo
		.setWaitSemaphores(waitSems) //
		.setWaitDstStageMask(waitStage)
		.setSignalSemaphores(signalSems)
		.setCommandBuffers(bufs);

	Device->graphicsQueue.submit(1u, &submitInfo, *m_frameFence[m_currentFrame]);

	std::array swaps{ mainSwapchain->handle() };

	vk::PresentInfoKHR presentInfo{};
	presentInfo //
		.setWaitSemaphores(m_renderFinishedSem[m_currentFrame].get())
		.setSwapchains(swaps)
		.setImageIndices(imageIndex);

	Device->presentQueue.presentKHR(presentInfo);
}
} // namespace vl
