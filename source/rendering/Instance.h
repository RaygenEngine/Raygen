#pragma once
#include "rendering/wrappers/RPhysicalDevice.h"

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace vl {
inline struct Instance_ : public vk::Instance {

	vk::SurfaceKHR surface;

	vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger;

	std::vector<RPhysicalDevice> capablePhysicalDevices;

	Instance_(const std::vector<const char*>&& requiredExtensions, GLFWwindow* window);
	~Instance_();
} * Instance{};
} // namespace vl


void InitVulkanLoader();
