#include "pch/pch.h"

#include "renderer/renderers/vulkan/VkRendererBase.h"
#include "system/Logger.h"

#include <set>

#include <vulkan/vulkan_win32.h>

#define vkCall(x)                                                                                                      \
	do {                                                                                                               \
		CLOG_ABORT(x != VK_SUCCESS, "Failed vkCall");                                                                  \
	} while (0)


#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif // !NDEBUG

namespace {
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			LOG_DEBUG("Validation layer: {}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			LOG_INFO("Validation layer: {}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LOG_WARN("Validation layer: {}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LOG_ERROR("Validation layer: {}", pCallbackData->pMessage);
			break;
		default: break;
	}

	return VK_FALSE;
}

void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
								 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
								 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
							 | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
							 | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
}

std::vector<const char*> requiredExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


bool CheckIfDeviceExtensionsAreAvailable(VkPhysicalDevice device)
{
	uint32 extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredDeviceExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredDeviceExtensions.erase(extension.extensionName);
	}

	return requiredDeviceExtensions.empty();
}

bool CheckNeededDeviceProperties(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
}

void CheckIfExtensionsAreAvailable()
{
	uint32 count;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(count);
	vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions.data());

	for (const char* extensionName : requiredExtensions) {
		auto it = std::find_if(availableExtensions.begin(), availableExtensions.end(),
			[extensionName](
				const VkExtensionProperties& props) { return strcmp(extensionName, props.extensionName) == 0; });

		// didn't find extension
		if (it == availableExtensions.end()) {
			LOG_ABORT("Missing required Vulkan extension: {}", extensionName);
			break;
		}
	}
}

void CheckIfValidationLayersAreAvailable()
{
	uint32 count;
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	std::vector<VkLayerProperties> availableLayers(count);
	vkEnumerateInstanceLayerProperties(&count, availableLayers.data());

	for (const char* layerName : validationLayers) {
		auto it = std::find_if(availableLayers.begin(), availableLayers.end(),
			[layerName](const VkLayerProperties& props) { return strcmp(layerName, props.layerName) == 0; });

		// didn't find layer
		if (it == availableLayers.end()) {
			LOG_ABORT("Missing required Vulkan layer: {}", layerName);
			break;
		}
	}
}


vk::SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	vk::SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

vk::QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	vk::QueueFamilyIndices indices;

	// Assign index to queue families that could be found
	uint32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	uint32 i = 0;
	for (const auto& queueFamily : queueFamilies) {

		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	// search for req format
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	// if search fails just return the first available
	return availableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		// if available, triple buffering approach
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		// TODO:
		VkExtent2D actualExtent = { 1920, 1080 };

		actualExtent.width = std::max(
			capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(
			capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

bool CheckSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	vk::SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, surface);
	return !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
}

bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	vk::QueueFamilyIndices indices = FindQueueFamilies(device, surface);

	// queue families properties and extensions are ok
	return indices.isComplete() && CheckNeededDeviceProperties(device) && CheckIfDeviceExtensionsAreAvailable(device)
		   && CheckSwapChainSupport(device, surface);
}

// TODO: Rate device suitability and chose the count of gpus with the best scores based on properties etc
VkPhysicalDevice FindAnySuitableDevice(VkInstance instance, VkSurfaceKHR surface)
{
	uint32 deviceCount;
	vkCall(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

	CLOG_ABORT(deviceCount == 0, "Failed to find GPUs with Vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// Choose device based on properties
	VkPhysicalDevice chosenDevice = VK_NULL_HANDLE;
	for (const auto& device : devices) {
		if (IsDeviceSuitable(device, surface)) {
			chosenDevice = device;
			break;
		}
	}

	CLOG_ABORT(!chosenDevice, "Failed to find gpu for vulkan rendering");
	return chosenDevice;
};


} // namespace

namespace vk {

VkRendererBase::~VkRendererBase()
{
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	}

	for (auto imageView : m_swapChainImageViews) {
		vkDestroyImageView(m_device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

	vkDestroyDevice(m_device, nullptr);

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	vkDestroyInstance(m_instance, nullptr);
}

void VkRendererBase::CreateInstance()
{
	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.pApplicationName = "KaleidoApp";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "KaleidoEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appInfo;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	PopulateDebugMessengerCreateInfo(debugCreateInfo);

	if constexpr (enableValidationLayers) {

		CheckIfValidationLayersAreAvailable();

		createInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		// vk init and destroy debug messenger
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}

	CheckIfExtensionsAreAvailable();

	createInfo.enabledExtensionCount = static_cast<uint32>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	vkCall(vkCreateInstance(&createInfo, nullptr, &m_instance));

	// Create debug messenger
	if constexpr (enableValidationLayers) {
		vkCall(CreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger));
	}
}

void VkRendererBase::CreateSurface(HWND assochWnd, HINSTANCE instance)
{
	VkWin32SurfaceCreateInfoKHR win32SurfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	win32SurfaceInfo.hwnd = assochWnd;
	win32SurfaceInfo.hinstance = instance;

	vkCall(vkCreateWin32SurfaceKHR(m_instance, &win32SurfaceInfo, nullptr, &m_surface));
}

void VkRendererBase::CreateDevice(VkPhysicalDevice physicalDevice, const QueueFamilyIndices& indices)
{
	VkDeviceQueueCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	createInfo.queueFamilyIndex = indices.graphicsFamily.value();
	createInfo.queueCount = 1;
	float qp0 = 1.0f;
	createInfo.pQueuePriorities = &qp0;

	// TODO:
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceCreateInfo.pQueueCreateInfos = &createInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	if constexpr (enableValidationLayers) {
		deviceCreateInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	vkCall(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &m_device), "failed to create logical device!");

	// Get device's graphics queue
	vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);

	// Get device's presentation queue
	std::vector<VkDeviceQueueCreateInfo> createInfos;
	std::set<uint32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float qp1 = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		createInfo.queueFamilyIndex = queueFamily;
		createInfo.queueCount = 1;
		createInfo.pQueuePriorities = &qp1;
		createInfos.push_back(createInfo);
	}

	vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

void VkRendererBase::CreateSwapChain(VkPhysicalDevice physicalDevice, const QueueFamilyIndices& indices)
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, m_surface);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

	// Store swap chain image format and extent
	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;

	uint32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = m_surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	vkCall(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain), "failed to create swap chain!");

	// Get swapchain's images
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());
}

void VkRendererBase::CreateSwapChainImageViews()
{
	m_swapChainImageViews.resize(m_swapChainImages.size());

	for (size_t i = 0; i < m_swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		createInfo.image = m_swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		vkCall(vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]),
			"failed to create image views!");
	}
}

void VkRendererBase::Init(HWND assochWnd, HINSTANCE instance)
{
	// Vk instance and debug messenger
	CreateInstance();

	// Vk surface
	CreateSurface(assochWnd, instance);

	// Physical device and queue family indices (graphics and presentation)
	auto physicalDevice = FindAnySuitableDevice(m_instance, m_surface);
	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, m_surface);

	// Logical device (graphics and presentation queues)
	CreateDevice(physicalDevice, indices);

	// Swapchain images
	CreateSwapChain(physicalDevice, indices);

	// Swapchain image views
	CreateSwapChainImageViews();

} // namespace vk

bool VkRendererBase::SupportsEditor()
{
	return false;
}

void VkRendererBase::Render()
{
}

} // namespace vk
