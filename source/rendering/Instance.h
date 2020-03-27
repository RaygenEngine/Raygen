#pragma once
#include "rendering/wrapper/PhysicalDevice.h"

// CHECK:
#include <glfw/glfw3.h>
#include <vulkan/vulkan.hpp>


namespace vl {

// Instance layer wrapper
inline struct S_Instance : public vk::Instance {

	vk::SurfaceKHR surface;

	vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger;

	std::vector<UniquePtr<PhysicalDevice>> capablePhysicalDevices;

	S_Instance(std::vector<const char*> requiredExtensions, GLFWwindow* window);
	~S_Instance();
} * Instance;


} // namespace vl
