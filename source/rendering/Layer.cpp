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
#include "rendering/wrappers/RSwapchain.h"

#include "universe/Universe.h"
#include "engine/Input.h"
#include "ecs_universe/systems/SceneCmdSystem.h"
#include <editor\EditorObject.h>
#include "rendering/scene/SceneSpotlight.h"

ConsoleFunction<> console_BuildAll{ "s.buildAll", []() { vl::Layer->mainScene->BuildAll(); },
	"Builds all build-able scene nodes" };
ConsoleFunction<> console_BuildAS{ "s.buildTestAccelerationStructure", []() {},
	"Builds a top level acceleration structure, for debugging purposes, todo: remove" };

namespace vl {


Layer_::Layer_()
{
	VulkanLoader::InitLoaderBase();

	Instance = new Instance_(Platform::GetVulkanExtensions(), Platform::GetMainHandle());

	// TODO: extension loading from pd structs in pNext*
	const auto deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME,
		// ray tracing device extensions
		VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
		VK_KHR_MAINTENANCE3_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
		VK_KHR_RAY_TRACING_EXTENSION_NAME,
	};

	Device = new Device_(Instance->capablePhysicalDevices[0], deviceExtensions);

	GpuResources = new GpuResources_();
	GpuAssetManager = new GpuAssetManager_();

	Layouts = new Layouts_();


	mainSwapchain = std::make_unique<RSwapchain>(Instance->surface);


	// TODO: scene is not a global
	mainScene = std::make_unique<Scene>();
	secondScene = std::make_unique<Scene>();
	currentScene = mainScene.get();

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
	allocInfo.setCommandPool(Device->mainCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(c_framesInFlight);

	// allocate all buffers needed
	{
		auto buffers = Device->allocateCommandBuffers(allocInfo);

		auto moveBuffersToArray = [&buffers](auto& target, size_t index) {
			auto begin = buffers.begin() + (index * c_framesInFlight);
			std::move(begin, begin + c_framesInFlight, target.begin());
		};

		moveBuffersToArray(m_cmdBuffer, 0);
	}


	Event::OnWindowResize.BindFlag(this, m_didWindowResize);
	Event::OnWindowMinimize.Bind(this, [&](bool newIsMinimized) { m_isMinimized = newIsMinimized; });


} // namespace vl

Layer_::~Layer_()
{
	delete Renderer;

	// WIP:
	mainSwapchain.reset();
	secondSwapchain.reset();
	mainScene.reset();
	secondScene.reset();

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
	if (Input.IsDown(Key::Tab)) {
		currentScene = secondScene.get();
	}
	else {
		currentScene = mainScene.get();
	}

	if (*m_didWindowResize) {
		Device->waitIdle();

		mainSwapchain.reset();
		mainSwapchain = std::make_unique<RSwapchain>(Instance->surface);
	}

	if (m_isMinimized)
		[[unlikely]] { return; }

	Renderer->PrepareForFrame();

	GpuAssetManager->ConsumeAssetUpdates();

	currentScene->ConsumeCmdQueue();

	currentFrame = (currentFrame + 1) % c_framesInFlight;
	auto currentCmdBuffer = &m_cmdBuffer[currentFrame];

	Device->waitForFences({ *m_inFlightFence[currentFrame] }, true, UINT64_MAX);
	Device->resetFences({ *m_inFlightFence[currentFrame] });

	currentScene->UploadDirty(currentFrame);

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

	Device->mainQueue.submit(1u, &submitInfo, *m_inFlightFence[currentFrame]);

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
