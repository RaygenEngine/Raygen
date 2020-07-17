#include "pch.h"
#include "Layer.h"

#include "platform/Platform.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Instance.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/Swapchain.h"
#include "rendering/resource/GpuResources.h"

#include "rendering/VulkanLoader.h"
#include "rendering/Device.h"

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
	Renderer->DrawFrame();
}
} // namespace vl
