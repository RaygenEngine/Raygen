#pragma once
#include "rendering/wrapper/PhysicalDevice.h"

// CHECK:
#include <glfw/glfw3.h>
#include <vulkan/vulkan.hpp>


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
