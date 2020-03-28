#pragma once
#include "rendering/wrapper/PhysicalDevice.h"

#include <vulkan/vulkan.hpp>

struct GLFWwindow;
namespace vl {

// Instance layer wrapper
inline struct Instance_ : public vk::Instance {

	vk::SurfaceKHR surface;

	vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger;

	std::vector<UniquePtr<PhysicalDevice>> capablePhysicalDevices;

	Instance_(std::vector<const char*> requiredExtensions, GLFWwindow* window);
	~Instance_();
} * Instance{};


} // namespace vl
