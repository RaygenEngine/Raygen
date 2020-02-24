#include "pch/pch.h"

#include "renderer/InstanceWrapper.h"

#include "system/Logger.h"

#include <glfw/glfw3.h>


#define vkCall(x)                                                                                                      \
	do {                                                                                                               \
		CLOG_ABORT(x != VK_SUCCESS, "Failed vkCall");                                                                  \
	} while (0)

PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pMessenger)
{
	return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
	VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const* pAllocator)
{
	return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

namespace {
VkBool32 DebugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes, VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
	void* /*pUserData*/)
{
	std::string message;

	message += vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) + ": "
			   + vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) + ":\n";
	message += std::string("\t") + "messageIDName   = <" + pCallbackData->pMessageIdName + ">\n";
	message += std::string("\t") + "messageIdNumber = " + std::to_string(pCallbackData->messageIdNumber) + "\n";
	message += std::string("\t") + "message         = <" + pCallbackData->pMessage + ">\n";
	if (0 < pCallbackData->queueLabelCount) {
		message += std::string("\t") + "Queue Labels:\n";
		for (uint8_t i = 0; i < pCallbackData->queueLabelCount; i++) {
			message += std::string("\t\t") + "labelName = <" + pCallbackData->pQueueLabels[i].pLabelName + ">\n";
		}
	}
	if (0 < pCallbackData->cmdBufLabelCount) {
		message += std::string("\t") + "CommandBuffer Labels:\n";
		for (uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
			message += std::string("\t\t") + "labelName = <" + pCallbackData->pCmdBufLabels[i].pLabelName + ">\n";
		}
	}
	if (0 < pCallbackData->objectCount) {
		for (uint8_t i = 0; i < pCallbackData->objectCount; i++) {
			message += std::string("\t") + "Object " + std::to_string(i) + "\n";
			message += std::string("\t\t") + "objectType   = "
					   + vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) + "\n";
			message += std::string("\t\t") + "objectHandle = " + std::to_string(pCallbackData->pObjects[i].objectHandle)
					   + "\n";
			if (pCallbackData->pObjects[i].pObjectName) {
				message += std::string("\t\t") + "objectName   = <" + pCallbackData->pObjects[i].pObjectName + ">\n";
			}
		}
	}

	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: LOG_INFO("{}", message.c_str()); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: LOG_WARN("{}", message.c_str()); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: LOG_ERROR("{}", message.c_str()); break;
	}

	return false;
}

bool CheckLayers(std::vector<char const*> const& layers, std::vector<vk::LayerProperties> const& properties)
{
	// return true if all layers are listed in the properties
	return std::all_of(layers.begin(), layers.end(), [&properties](char const* name) {
		return std::find_if(properties.begin(), properties.end(), [&name](vk::LayerProperties const& property) {
			return strcmp(property.layerName, name) == 0;
		}) != properties.end();
	});
}


std::vector<const char*> requiredExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
} // namespace


void InstanceWrapper::Init(std::vector<const char*> additionalExtensions, WindowType* window)
{
	std::vector<vk::LayerProperties> instanceLayerProperties = vk::enumerateInstanceLayerProperties();

	/* VULKAN_KEY_START */

	// Use standard_validation meta layer that enables all recommended validation layers
	std::vector<char const*> instanceLayerNames = { "VK_LAYER_KHRONOS_validation" };

	if (!CheckLayers(instanceLayerNames, instanceLayerProperties)) {
		LOG_ABORT("Set the environment variable VK_LAYER_PATH to point to the location of your layers");
	}

	for (auto& extension : additionalExtensions) {
		requiredExtensions.push_back(extension);
	}

	// create instance
	vk::ApplicationInfo appInfo{};
	appInfo.setPApplicationName("KaleidoApp")
		.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
		.setPEngineName("KaleidoEngine")
		.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
		.setApiVersion(VK_API_VERSION_1_2);

	vk::InstanceCreateInfo createInfo{};
	createInfo.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(static_cast<uint32>(requiredExtensions.size()))
		.setPpEnabledExtensionNames(requiredExtensions.data());

	createInfo.setEnabledLayerCount(static_cast<uint32>(instanceLayerNames.size()))
		.setPpEnabledLayerNames(instanceLayerNames.data());


	m_vkHandle = vk::createInstanceUnique(createInfo);


	pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		m_vkHandle->getProcAddr("vkCreateDebugUtilsMessengerEXT"));
	if (!pfnVkCreateDebugUtilsMessengerEXT) {
		LOG_ABORT("GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function.");
	}

	pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		m_vkHandle->getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
	if (!pfnVkDestroyDebugUtilsMessengerEXT) {
		LOG_ABORT("GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function.");
	}

	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
	vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
													   | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
													   | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
	m_debugUtilsMessenger = m_vkHandle->createDebugUtilsMessengerEXTUnique(
		vk::DebugUtilsMessengerCreateInfoEXT({}, severityFlags, messageTypeFlags, &DebugMessageFunc));

	// create surface (WIP: currently C form)

	VkSurfaceKHR tmp;
	vkCall(glfwCreateWindowSurface(m_vkHandle.get(), window, nullptr, &tmp));
	m_surface = tmp;

	// get capable physical devices
	auto deviceHandles = m_vkHandle->enumeratePhysicalDevices();

	for (const auto dH : deviceHandles) {
		PhysicalDeviceWrapper pd{};
		pd.Init(dH, m_surface);
		// if capable
		if (pd.GetDeviceRating() > 0) {
			m_capablePhysicalDevices.push_back(pd);
		}
	}
}

InstanceWrapper::~InstanceWrapper()
{
	m_vkHandle->destroySurfaceKHR(m_surface);
}

PhysicalDeviceWrapper& InstanceWrapper::GetBestCapablePhysicalDevice()
{
	CLOG_ABORT(m_capablePhysicalDevices.empty(), "No capable physical device found for required vulkan rendering");

	// WIP:
	// auto it = std::max_element(m_capablePhysicalDevices.begin(), m_capablePhysicalDevices.end(),
	//	[](std::unique_ptr<PhysicalDevice> a, std::unique_ptr<PhysicalDevice> b) {
	//		return a->GetDeviceRating() < b->GetDeviceRating();
	//	});

	// return { it->get() };

	return m_capablePhysicalDevices[0];
}
