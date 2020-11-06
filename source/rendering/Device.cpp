#include "Device.h"

#include "rendering/VulkanLoader.h"

#include <set>

namespace {
void CheckExtensions(std::vector<char const*> const& extensions, std::vector<vk::ExtensionProperties> const& properties)
{
	std::for_each(extensions.begin(), extensions.end(), [&properties](char const* name) {
		auto found = std::find_if(properties.begin(), properties.end(),
						 [&name](vk::ExtensionProperties const& property) {
							 return strcmp(property.extensionName, name) == 0;
						 })
					 != properties.end();
		CLOG_ABORT(!found, "Requested Vulkan device extension not found: {}", name);
		return found;
	});
}
} // namespace

namespace vl {
Device_::Device_(RPhysicalDevice& pd)
	: pd(pd)
{
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32> uniqueQueueFamilies = {
		pd.graphicsFamily.index,
		pd.dmaFamily.index,
		pd.computeFamily.index,
		pd.presentFamily.index,
	};

	std::array qps{ 1.0f };
	vk::DeviceQueueCreateInfo createInfo{};
	for (uint32 queueFamily : uniqueQueueFamilies) {
		vk::DeviceQueueCreateInfo createInfo{};
		createInfo
			.setQueueFamilyIndex(queueFamily) //
			.setQueuePriorities(qps);
		queueCreateInfos.push_back(createInfo);
	}

	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceBufferDeviceAddressFeatures,
		vk::PhysicalDeviceDescriptorIndexingFeatures, vk::PhysicalDeviceRayTracingFeaturesKHR>
		pDeviceFeaturesChain;

	auto& deviceFeatures = pDeviceFeaturesChain.get<vk::PhysicalDeviceFeatures2>();
	auto& deviceBufferAddressFeatures = pDeviceFeaturesChain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>();
	auto& deviceRayTracingFeatures = pDeviceFeaturesChain.get<vk::PhysicalDeviceRayTracingFeaturesKHR>();

	deviceFeatures.features.setSamplerAnisotropy(VK_TRUE);
	deviceFeatures.features.setFragmentStoresAndAtomics(VK_TRUE);
	deviceFeatures.features.setFillModeNonSolid(VK_TRUE);
	deviceBufferAddressFeatures.setBufferDeviceAddress(VK_TRUE);

	pDeviceFeaturesChain.get<vk::PhysicalDeviceDescriptorIndexingFeatures>()
		.setRuntimeDescriptorArray(true) //
		.setShaderSampledImageArrayNonUniformIndexing(true)
		.setDescriptorBindingVariableDescriptorCount(true);

	// get all available rt extensions from gpu
	// careful pNext here is lost
	deviceRayTracingFeatures = pd.rtFeats;


	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
		VK_KHR_RAY_TRACING_EXTENSION_NAME,
	};

	CheckExtensions(deviceExtensions, pd.enumerateDeviceExtensionProperties());

	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo
		.setQueueCreateInfos(queueCreateInfos) //
		.setPEnabledExtensionNames(deviceExtensions)
		.setPNext(&deviceFeatures);

	vk::Device::operator=(pd.createDevice(deviceCreateInfo));
	VulkanLoader::InitLoaderWithDevice(*this);
}

uint32 Device_::FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties) const
{
	vk::PhysicalDeviceMemoryProperties memProperties = pd.getMemoryProperties();

	for (uint32 i = 0; i < memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	LOG_ABORT("Failed to find suitable memory type!");
}

vk::Format Device_::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, //
	const vk::FormatFeatureFlags features) const
{
	for (auto format : candidates) {
		vk::FormatProperties props = pd.getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	LOG_ABORT("Failed to find supported format!");
}

vk::Format Device_::FindDepthFormat() const
{
	return FindSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::Format Device_::FindDepthStencilFormat() const
{
	return FindSupportedFormat(
		{ vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint },
		vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}


SwapchainSupportDetails Device_::GetSwapchainSupportDetails() const
{
	SwapchainSupportDetails ssDetails;

	ssDetails.capabilities = pd.getSurfaceCapabilitiesKHR(pd.surface);
	ssDetails.formats = pd.getSurfaceFormatsKHR(pd.surface);
	ssDetails.presentModes = pd.getSurfacePresentModesKHR(pd.surface);
	return ssDetails;
}

Device_::~Device_()
{
	destroy();
}

CmdPoolManager_::CmdPoolManager_()
	// CHECK: yikes
	: graphicsQueue(Device->pd.graphicsFamily)
	, dmaQueue(Device->pd.dmaFamily)
	, computeQueue(Device->pd.computeFamily)
	, presentQueue(Device->pd.presentFamily)
	, graphicsCmdPool(graphicsQueue)
	, dmaCmdPool(dmaQueue)
	, computeCmdPool(computeQueue)
{
	DEBUG_NAME(graphicsQueue, "Graphics Queue");
	DEBUG_NAME(dmaQueue, "Dma Queue");
	DEBUG_NAME(computeQueue, "Compute Queue");
	DEBUG_NAME(presentQueue, "Present Queue");
}
} // namespace vl
