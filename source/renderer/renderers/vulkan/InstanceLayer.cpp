#include "pch/pch.h"

#include "renderer/renderers/vulkan/InstanceLayer.h"

#include "system/Logger.h"

#include <glfw/glfw3.h>


#define vkCall(x)                                                                                                      \
	do {                                                                                                               \
		auto r = x;                                                                                                    \
		CLOG_ABORT(r != VK_SUCCESS, "Failed vkCall: {} error: {}", ##x, r);                                            \
	} while (0)

namespace vlkn {
std::vector<const char*> requiredExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

InstanceLayer::InstanceLayer(std::vector<const char*> additionalExtensions, WindowType* window)
{
	// create instance
	vk::ApplicationInfo appInfo{};
	appInfo.setPApplicationName("RaygenApp")
		.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
		.setPEngineName("RaygenEngine")
		.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
		.setApiVersion(VK_API_VERSION_1_1);

	for (auto& extension : additionalExtensions) {
		requiredExtensions.push_back(extension);
	}

	vk::InstanceCreateInfo createInfo{};
	createInfo.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(static_cast<uint32>(requiredExtensions.size()))
		.setPpEnabledExtensionNames(requiredExtensions.data());

	m_instance = vk::createInstanceUnique(createInfo);


	VkSurfaceKHR tmp;
	vkCall(glfwCreateWindowSurface(m_instance.get(), window, nullptr, &tmp));
	m_surface = tmp;

	// get capable physical devices
	auto deviceHandles = m_instance->enumeratePhysicalDevices();

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
	m_instance->destroySurfaceKHR(m_surface);
	// m_instance->destroy();
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

} // namespace vlkn
