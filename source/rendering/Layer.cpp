#include "pch.h"
#include "Layer.h"

#include "platform/Platform.h"
#include "rendering/asset/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/renderer/Renderer.h"
#include "rendering/wrapper/Swapchain.h"


namespace vl {

Layer_::Layer_()
{
	auto requiredExtensions = Platform::GetVulkanExtensions();
	requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	Instance = new Instance_(requiredExtensions, Platform::GetMainHandle());


	auto deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME };
	Device = new Device_(Instance->capablePhysicalDevices[0].get(), deviceExtensions);

	GpuAssetManager = new S_GpuAssetManager();


	Renderer = new Renderer_();
	Renderer->Init();
}

Layer_::~Layer_()
{
	// ImguiImpl::CleanupVulkan(); // WIP:
	delete GpuAssetManager;


	delete Renderer;
	delete Device;
	delete Instance;
}


void Layer_::DrawFrame()
{
	Renderer->DrawFrame();
}

} // namespace vl
