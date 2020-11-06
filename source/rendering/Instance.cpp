#include "Instance.h"

#include "engine/console/ConsoleVariable.h"
#include "engine/Input.h"
#include "rendering/VulkanLoader.h"

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

ConsoleVariable<bool> showValidationErrors{ "r.validation.show", false, "Enables vulkan validation layer errors" };
ConsoleVariable<bool> validationBreakOnError{ "r.validation.breakOnError", false,
	"Breaks to allow the debugger to get a call stack." };

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
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
			LOG_ERROR("{}", message.c_str());
			if (validationBreakOnError) {
				LOG_ABORT("");
			}
			break;
		}
	}

	return false;
}

bool CheckLayersRemove(std::vector<char const*>& layers, std::vector<vk::LayerProperties> const& properties)
{
	std::erase_if(layers, [&properties](char const* name) {
		auto found = std::find_if(properties.begin(), properties.end(), [&name](vk::LayerProperties const& property) {
			return strcmp(property.layerName, name) == 0;
		}) != properties.end();
		CLOG_WARN(!found, "Requested Vulkan Validation layer not found: {}", name);
		return !found; // erase if not found
	});

	return layers.size() != 0;
}

void CheckExtensions(const std::vector<char const*>& extensions, std::vector<vk::ExtensionProperties> const& properties)
{
	std::for_each(extensions.begin(), extensions.end(), [&properties](char const* name) {
		auto found = std::find_if(properties.begin(), properties.end(),
						 [&name](vk::ExtensionProperties const& property) {
							 return strcmp(property.extensionName, name) == 0;
						 })
					 != properties.end();
		CLOG_ABORT(!found, "Requested Vulkan instance extension not found: {}", name);
		return found;
	});
}

} // namespace

namespace vl {
Instance_::Instance_(const std::vector<const char*>&& requiredExtensions, GLFWwindow* window)
{
	std::vector<char const*> instanceLayerNames = { "VK_LAYER_KHRONOS_validation" };
	const bool foundAnyLayers = CheckLayersRemove(instanceLayerNames, vk::enumerateInstanceLayerProperties());

	auto allExtensions = requiredExtensions;
	if (foundAnyLayers) {
		allExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	// ray tracing required extension
	allExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	CheckExtensions(allExtensions, vk::enumerateInstanceExtensionProperties());

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
		.setPEnabledExtensionNames(allExtensions);

	if (foundAnyLayers) {
		createInfo.setPEnabledLayerNames(instanceLayerNames);
	}

	vk::Instance::operator=(vk::createInstance(createInfo));

	VulkanLoader::InitLoaderWithInstance(*this);

	if (foundAnyLayers) {
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
		debugUtilsMessenger = createDebugUtilsMessengerEXT(
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
		physicalDevices.emplace_back(dH, surface);
	}


	if (Input.IsDown(Key::Shift)) {
		*validationBreakOnError = true;
	}
}

Instance_::~Instance_()
{
	destroySurfaceKHR(surface);
	if (debugUtilsMessenger) {
		destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
	}
	destroy();
}
} // namespace vl
