#pragma once
#include "rendering/objects/RPhysicalDevice.h"

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace vl {
inline struct Instance_ : public vk::Instance {

	vk::SurfaceKHR surface;

	vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger;

	std::vector<UniquePtr<RPhysicalDevice>> capablePhysicalDevices;

	Instance_(std::vector<const char*> requiredExtensions, GLFWwindow* window);
	~Instance_();
} * Instance{};
} // namespace vl
