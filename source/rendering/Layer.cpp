#include "pch.h"
#include "Layer.h"

#include "rendering/Instance.h"
#include "rendering/Device.h"
#include "rendering/wrapper/Swapchain.h"
#include "rendering/asset/GpuAssetManager.h"
#include "rendering/renderer/Renderer.h"
#include "platform/Platform.h"


namespace vl {

S_Layer::S_Layer()
{
	auto requiredExtensions = Platform::GetVulkanExtensions();
	requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	Instance = new S_Instance(requiredExtensions, Platform::GetMainHandle());


	auto deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME };
	Device = new S_Device(Instance->capablePhysicalDevices[0].get(), deviceExtensions);

	GpuAssetManager = new S_GpuAssetManager();


	Renderer = new S_Renderer();
	Renderer->Init();
}

S_Layer::~S_Layer()
{
	// ImguiImpl::CleanupVulkan(); // WIP:
	delete GpuAssetManager;


	delete Renderer;
	delete Device;
	delete Instance;
}


void S_Layer::DrawFrame()
{
	Renderer->DrawFrame();
}

} // namespace vl
