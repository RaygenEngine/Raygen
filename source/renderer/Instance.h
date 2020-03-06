#pragma once

#include "engine/Engine.h"
#include "renderer/PhysicalDevice.h"

#include <vulkan/vulkan.hpp>


// Instance layer wrapper
struct Instance : public vk::Instance {

	vk::SurfaceKHR surface;

	vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger;

	std::vector<std::unique_ptr<PhysicalDevice>> capablePhysicalDevices;

	Instance(std::vector<const char*> requiredExtensions, WindowType* window);
	~Instance();
};
