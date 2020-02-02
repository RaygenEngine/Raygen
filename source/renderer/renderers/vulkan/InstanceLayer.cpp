#include "pch/pch.h"

#include "renderer/renderers/vulkan/InstanceLayer.h"

#include "system/Logger.h"

#include <vulkan/vulkan_win32.h>


#define vkCall(x)                                                                                                      \
	do {                                                                                                               \
		CLOG_ABORT(x != VK_SUCCESS, "Failed vkCall");                                                                  \
	} while (0)

namespace vulkan {
std::vector<const char*> requiredExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

InstanceLayer::InstanceLayer(HWND assochWnd, HINSTANCE instance)
{
	// create instance
	vk::ApplicationInfo appInfo{};
	appInfo.setPApplicationName("KaleidoApp")
		.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
		.setPEngineName("KaleidoEngine")
		.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
		.setApiVersion(VK_API_VERSION_1_1);

	vk::InstanceCreateInfo createInfo{};
	createInfo.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(static_cast<uint32>(requiredExtensions.size()))
		.setPpEnabledExtensionNames(requiredExtensions.data());

	m_instance = vk::createInstance(createInfo);

	// create surface (WIP: currently C form)
	VkWin32SurfaceCreateInfoKHR win32SurfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	win32SurfaceInfo.hwnd = assochWnd;
	win32SurfaceInfo.hinstance = instance;

	VkSurfaceKHR tmp;
	vkCall(vkCreateWin32SurfaceKHR(m_instance, &win32SurfaceInfo, nullptr, &tmp));
	m_surface = tmp;

	// get capable physical devices
	auto deviceHandles = m_instance.enumeratePhysicalDevices();

	for (const auto dH : deviceHandles) {
		auto pD = std::make_unique<PhysicalDevice>(dH, m_surface);
		// if capable
		if (pD->GetDeviceRating() > 0) {
			m_capablePhysicalDevices.push_back(std::move(pD));
		}
	}
}

InstanceLayer::~InstanceLayer()
{
	m_instance.destroySurfaceKHR(m_surface);
	m_instance.destroy();
}

PhysicalDevice* InstanceLayer::GetBestCapablePhysicalDevice()
{
	CLOG_ABORT(m_capablePhysicalDevices.empty(), "No capable physical device found for required vulkan rendering");

	// WIP:
	// auto it = std::max_element(m_capablePhysicalDevices.begin(), m_capablePhysicalDevices.end(),
	//	[](std::unique_ptr<PhysicalDevice> a, std::unique_ptr<PhysicalDevice> b) {
	//		return a->GetDeviceRating() < b->GetDeviceRating();
	//	});

	// return { it->get() };

	return m_capablePhysicalDevices[0].get();
}

} // namespace vulkan
