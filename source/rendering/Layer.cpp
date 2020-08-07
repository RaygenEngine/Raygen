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

ConsoleFunction<> console_BuildAll{ "s.buildAll", []() { vl::Layer->mainScene->BuildAll(); },
	"Builds all build-able scene nodes" };
ConsoleFunction<> console_BuildAS{ "s.buildTestAccelerationStructure", []() {},
	"Builds a top level acceleration structure, for debugging purposes, todo: remove" };

namespace vl {

Layer_::Layer_()
{
	VulkanLoader::InitLoaderBase();

	Instance = new Instance_(Platform::GetVulkanExtensions(), Platform::GetMainHandle());

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

	Device = new Device_(Instance->capablePhysicalDevices[0].get(), deviceExtensions);

	GpuResources = new GpuResources_();
	GpuAssetManager = new GpuAssetManager_();

	Layouts = new Layouts_();


	mainSwapchain = std::make_unique<RSwapchain>(Instance->surface);
	// TODO: scene is not a global
	mainScene = std::make_unique<Scene>(mainSwapchain->imageCount);

	Renderer = new Renderer_();
	Renderer->InitPipelines();
} // namespace vl

Layer_::~Layer_()
{
	delete Renderer;

	// WIP:
	mainSwapchain.reset();
	mainScene.reset();

	delete Layouts;

	delete GpuAssetManager;
	delete GpuResources;

	delete Device;
	delete Instance;
}

void Layer_::DrawFrame()
{
	// this should be the general approach
	// even for more than one renderers/scenes/swapchains

	// if window resize
	// Device->waitIdle();
	// delete Swapchain;
	// Swapchain = new Swapchain_(Instance->surface);

	// if(window not minimized)

	Renderer->UpdateForFrame();
	GpuAssetManager->ConsumeAssetUpdates();
	mainScene->ConsumeCmdQueue();

	auto imageAvailSem = Renderer->PrepareForDraw();

	mainScene->UploadDirty();

	uint32 imageIndex;
	Device->acquireNextImageKHR(*mainSwapchain, UINT64_MAX, { imageAvailSem }, {}, &imageIndex);

	auto outRp = mainSwapchain->renderPass.get();
	auto outFb = mainSwapchain->framebuffers[imageIndex].get();
	auto outExtent = mainSwapchain->extent;

	// WIP: 1 = editor camera (lets hope for now)
	SceneRenderDesc sceneDesc{ mainScene.get(), 1 };

	auto renderFinishedSem = Renderer->DrawFrame(sceneDesc, outRp, outFb, outExtent);

	vk::SwapchainKHR swapChains[] = { *mainSwapchain };

	vk::PresentInfoKHR presentInfo;
	presentInfo //
		.setWaitSemaphoreCount(1u)
		.setPWaitSemaphores(&renderFinishedSem)
		.setSwapchainCount(1u)
		.setPSwapchains(swapChains)
		.setPImageIndices(&imageIndex)
		.setPResults(nullptr);

	Device->presentQueue.presentKHR(presentInfo);
}
} // namespace vl
