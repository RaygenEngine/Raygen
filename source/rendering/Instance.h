#pragma once
#include "rendering/wrappers/PhysicalDevice.h"

struct GLFWwindow;

namespace vl {
inline struct Instance_ : public vk::Instance {

	vk::SurfaceKHR surface;

	vk::DebugUtilsMessengerEXT debugUtilsMessenger;

	std::vector<RPhysicalDevice> physicalDevices;

	Instance_(const std::vector<const char*>&& requiredExtensions, GLFWwindow* window);
	~Instance_();
} * Instance{};
} // namespace vl


void InitVulkanLoader();
