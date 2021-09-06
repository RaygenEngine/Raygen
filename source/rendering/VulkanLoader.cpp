#include "VulkanLoader.h"

#include "rendering/VkCoreIncludes.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

#include <Windows.h>

namespace vk {
class DynamicLoader {
public:
#ifdef VULKAN_HPP_NO_EXCEPTIONS
	DynamicLoader() VULKAN_HPP_NOEXCEPT : m_success(false)
#else
	DynamicLoader()
		: m_success(false)
#endif
	{
#if defined(__linux__)
		m_library = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
#elif defined(__APPLE__)
		m_library = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
#elif defined(_WIN32)
		m_library = LoadLibrary(TEXT("vulkan-1.dll"));
#else
		assert(false && "unsupported platform");
#endif

		m_success = m_library != 0;
#ifndef VULKAN_HPP_NO_EXCEPTIONS
		if (!m_success) {
			// NOTE there should be an InitializationFailedError, but msvc insists on the symbol does not exist within
			// the scope of this function.
			throw std::runtime_error("Failed to load vulkan library!");
		}
#endif
	}

	~DynamicLoader() VULKAN_HPP_NOEXCEPT
	{
		if (m_library) {
#if defined(__linux__) || defined(__APPLE__)
			dlclose(m_library);
#elif defined(_WIN32)
			FreeLibrary(m_library);
#endif
		}
	}

	template<typename T>
	T getProcAddress(const char* function) const VULKAN_HPP_NOEXCEPT
	{
#if defined(__linux__) || defined(__APPLE__)
		return (T)dlsym(m_library, function);
#elif defined(_WIN32)
		return (T)GetProcAddress(m_library, function);
#endif
	}

	bool success() const VULKAN_HPP_NOEXCEPT { return m_success; }

private:
	bool m_success;
#if defined(__linux__) || defined(__APPLE__)
	void* m_library;
#elif defined(_WIN32)
	HMODULE m_library;
#else
#	error unsupported platform
#endif
};
} // namespace vk

vk::DynamicLoader dl;

void InitVulkanLoader()
{
	vk::Instance instance = vk::createInstance({}, nullptr);

	// create a dispatcher, based on additional vkDevice/vkGetDeviceProcAddr
	std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
	assert(!physicalDevices.empty());

	vk::Device device = physicalDevices[0].createDevice({}, nullptr);

	// function pointer specialization for device
	VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
}

void VulkanLoader::InitLoaderBase()
{
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
		= dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
}

void VulkanLoader::InitLoaderWithInstance(vk::Instance& instance)
{
	// initialize function pointers for instance
	VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
}

void VulkanLoader::InitLoaderWithDevice(vk::Device& device)
{
	// initialize function pointers for instance
	VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
}
