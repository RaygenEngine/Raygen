#pragma once

#include "system/Engine.h"
#include "renderer/PhysicalDeviceWrapper.h"

#include <vulkan/vulkan.hpp>


// Instance layer wrapper
class InstanceWrapper : public VkUniqueObjectWrapper<vk::UniqueInstance> {
	// TODO: currently can't add as unique
	vk::SurfaceKHR m_surface;

	// WIP: change mem management here
	std::vector<PhysicalDeviceWrapper> m_capablePhysicalDevices;

	vk::UniqueDebugUtilsMessengerEXT m_debugUtilsMessenger;

public:
	void Init(std::vector<const char*> additionalExtensions, WindowType* window);
	~InstanceWrapper();

	vk::SurfaceKHR GetSurface() { return m_surface; }

	PhysicalDeviceWrapper& GetBestCapablePhysicalDevice();
};
