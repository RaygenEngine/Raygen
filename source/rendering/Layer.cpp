#include "pch.h"
#include "Layer.h"

#include "platform/Platform.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/scene/Scene.h"
#include "rendering/Swapchain.h"
#include "rendering/VulkanLoader.h"

namespace vl {
Layer_::Layer_()
{
	auto requiredExtensions = Platform::GetVulkanExtensions();
	requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	VulkanLoader::InitLoaderBase();

	Instance = new Instance_(requiredExtensions, Platform::GetMainHandle());

	auto deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME };
	Device = new Device_(Instance->capablePhysicalDevices[0].get(), deviceExtensions);

	GpuResources = new GpuResources_();
	GpuAssetManager = new GpuAssetManager_();

	Layouts = new Layouts_();

	Swapchain = new Swapchain_(Instance->surface);
	Scene = new Scene_(Swapchain->GetImageCount());

	Renderer = new Renderer_();
	Renderer->InitPipelines();
}

Layer_::~Layer_()
{
	delete Renderer;

	delete Swapchain;
	delete Scene;

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
	//Device->waitIdle();
	//delete Swapchain;
	//Swapchain = new Swapchain_(Instance->surface);

	// if(window not minimized)

	Renderer->UpdateForFrame();
	GpuAssetManager->ConsumeAssetUpdates();
	Scene->ConsumeCmdQueue();

	auto imageAvailSem = Renderer->PrepareForDraw();

	Scene->UploadDirty();

	uint32 imageIndex;
	Device->acquireNextImageKHR(*Swapchain, UINT64_MAX, { imageAvailSem }, {}, &imageIndex);
	
	auto outRp = Swapchain->GetRenderPass();
	auto outFb = Swapchain->GetFramebuffer(imageIndex);
	auto outExtent = Swapchain->GetExtent();

	// WIP: 1 = editor camera (lets hope for now)
	SceneRenderDesc<SceneCamera> sceneDesc{ Scene, 1 };

	auto renderFinishedSem = Renderer->DrawFrame(sceneDesc, outRp, outFb, outExtent);

	vk::SwapchainKHR swapChains[] = { *Swapchain };

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
