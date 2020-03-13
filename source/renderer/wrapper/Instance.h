#pragma once
#include "engine/Engine.h"
#include "renderer/wrapper/PhysicalDevice.h"

#include <vulkan/vulkan.hpp>

// Instance layer wrapper
struct Instance : public vk::Instance {

	vk::SurfaceKHR surface;

	vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger;

	std::vector<UniquePtr<PhysicalDevice>> capablePhysicalDevices;

	Instance(std::vector<const char*> requiredExtensions, GLFWwindow* window);
	~Instance();
};
