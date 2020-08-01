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

	Swapchain = new Swapchain_(Instance->surface);
	Scene = new Scene_(Swapchain->GetImageCount());

	Renderer = new Renderer_();
	Renderer->InitPipelines();
} // namespace vl

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
