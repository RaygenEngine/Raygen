#include "pch.h"
#include "renderer/wrapper/Instance.h"

#include "engine/Logger.h"
#include "engine/console/ConsoleVariable.h"

#include <glfw/glfw3.h>

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

ConsoleVariable<bool> showValidationErrors{ "r.showValidation", false, "Enables vulkan validation layer errors" };

namespace {
VkBool32 DebugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes, VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
	void* /*pUserData*/)
{
	if (!showValidationErrors) {
		return false;
	}

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
} // namespace

Instance::Instance(std::vector<const char*> requiredExtensions, GLFWwindow* window)
{
	auto allExtensions = requiredExtensions;
	allExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	allExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	std::vector<vk::LayerProperties> instanceLayerProperties = vk::enumerateInstanceLayerProperties();

	/* VULKAN_KEY_START */

	std::vector<char const*> instanceLayerNames = { "VK_LAYER_KHRONOS_validation" };

	const bool foundLayers = CheckLayers(instanceLayerNames, instanceLayerProperties);
	CLOG_WARN(!foundLayers, "Vulkan Validation layers not found.");


	// create instance
	vk::ApplicationInfo appInfo{};
	appInfo
		.setPApplicationName("RaygenApp") //
		.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
		.setPEngineName("RaygenEngine")
		.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
		.setApiVersion(VK_API_VERSION_1_2);

	vk::InstanceCreateInfo createInfo{};
	createInfo
		.setPApplicationInfo(&appInfo) //
		.setEnabledExtensionCount(static_cast<uint32>(allExtensions.size()))
		.setPpEnabledExtensionNames(allExtensions.data());

	if (foundLayers) {
		createInfo
			.setEnabledLayerCount(static_cast<uint32>(instanceLayerNames.size())) //
			.setPpEnabledLayerNames(instanceLayerNames.data());
	}

	vk::Instance::operator=(vk::createInstance(createInfo));

	if (foundLayers) {
		pfnVkCreateDebugUtilsMessengerEXT
			= reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(getProcAddr("vkCreateDebugUtilsMessengerEXT"));
		if (!pfnVkCreateDebugUtilsMessengerEXT) {
			LOG_ABORT("GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function.");
		}

		pfnVkDestroyDebugUtilsMessengerEXT
			= reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
		if (!pfnVkDestroyDebugUtilsMessengerEXT) {
			LOG_ABORT("GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function.");
		}

		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
		vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
														   | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
														   | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
		debugUtilsMessenger = createDebugUtilsMessengerEXTUnique(
			vk::DebugUtilsMessengerCreateInfoEXT({}, severityFlags, messageTypeFlags, &DebugMessageFunc));
	}

	VkSurfaceKHR tmp;
	if (glfwCreateWindowSurface(*this, window, nullptr, &tmp) != VK_SUCCESS) {
		LOG_ABORT("Failed to create glfw window surface");
	}
	surface = tmp;

	// get capable physical devices
	auto deviceHandles = enumeratePhysicalDevices();

	for (const auto dH : deviceHandles) {
		auto pd = std::make_unique<PhysicalDevice>(dH, surface);
		// if capable
		if (pd->rating > 0) {
			capablePhysicalDevices.push_back(std::move(pd));
		}
	}
}

Instance::~Instance()
{
	destroySurfaceKHR(surface);
	destroy();
}
