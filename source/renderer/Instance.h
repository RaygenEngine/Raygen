#pragma once

#include "system/Engine.h"
#include "renderer/PhysicalDevice.h"

#include <vulkan/vulkan.hpp>


// Instance layer wrapper
struct Instance {
	vk::UniqueInstance handle;
	vk::SurfaceKHR surface;

	vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger;

	std::vector<std::unique_ptr<PhysicalDevice>> capablePhysicalDevices;

	Instance(std::vector<const char*> requiredExtensions, WindowType* window);
	~Instance();
};
