#include "PhysicalDevice.h"

namespace {
void CheckExtensions(std::vector<char const*> const& extensions, std::vector<vk::ExtensionProperties> const& properties)
{
	std::for_each(extensions.begin(), extensions.end(), [&properties](char const* name) {
		auto found = std::find_if(properties.begin(), properties.end(),
						 [&name](vk::ExtensionProperties const& property) {
							 return strcmp(property.extensionName, name) == 0;
						 })
					 != properties.end();
		CLOG_ABORT(!found, "Vulkan extension not supported by device: {}", name);
		return found;
	});
}
} // namespace

#define CHECK_ADD_FEATURE(f)                                                                                           \
	CLOG_ABORT(!s_##f, "Feature not supported by device: {}", #f);                                                     \
	f = VK_TRUE;

namespace vl {
RPhysicalDevice::RPhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR inSurface)
	: vk::PhysicalDevice(vkHandle)
	, surface(inSurface)
{
	auto properties = getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPropertiesKHR>();
	generalProperties = properties.get<vk::PhysicalDeviceProperties2>();
	raytracingProperties = properties.get<vk::PhysicalDeviceRayTracingPropertiesKHR>();

	CLOG_ABORT(raytracingProperties.shaderGroupHandleSize != 32u && raytracingProperties.maxRecursionDepth < 1,
		"Rt properties not supported by device");

	auto supportedFeaturesChain = getFeatures2<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceBufferDeviceAddressFeatures, vk::PhysicalDeviceDescriptorIndexingFeatures,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceRayTracingFeaturesKHR>();

	auto& s_deviceFeatures = supportedFeaturesChain.get<vk::PhysicalDeviceFeatures2>();
	auto& s_bufferAddressFeatures = supportedFeaturesChain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>();
	auto& s_descriptorIndexingFeatures = supportedFeaturesChain.get<vk::PhysicalDeviceDescriptorIndexingFeatures>();
	auto& s_dynamicStateExtFeatures = supportedFeaturesChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
	auto& s_raytracingFeatures = supportedFeaturesChain.get<vk::PhysicalDeviceRayTracingFeaturesKHR>();

	auto& deviceFeatures = featuresChain.get<vk::PhysicalDeviceFeatures2>();
	auto& bufferAddressFeatures = featuresChain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>();
	auto& descriptorIndexingFeatures = featuresChain.get<vk::PhysicalDeviceDescriptorIndexingFeatures>();
	auto& dynamicStateExtFeatures = featuresChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
	auto& raytracingFeatures = featuresChain.get<vk::PhysicalDeviceRayTracingFeaturesKHR>();


	CHECK_ADD_FEATURE(deviceFeatures.features.samplerAnisotropy);
	CHECK_ADD_FEATURE(deviceFeatures.features.fragmentStoresAndAtomics);
	CHECK_ADD_FEATURE(deviceFeatures.features.fillModeNonSolid);
	CHECK_ADD_FEATURE(deviceFeatures.features.imageCubeArray);
	CHECK_ADD_FEATURE(bufferAddressFeatures.bufferDeviceAddress);
	CHECK_ADD_FEATURE(descriptorIndexingFeatures.runtimeDescriptorArray);
	CHECK_ADD_FEATURE(descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing);
	CHECK_ADD_FEATURE(descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount);
	CHECK_ADD_FEATURE(dynamicStateExtFeatures.extendedDynamicState);
	CHECK_ADD_FEATURE(raytracingFeatures.rayTracing);
	CHECK_ADD_FEATURE(raytracingFeatures.rayQuery);

	extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
		VK_KHR_RAY_TRACING_EXTENSION_NAME,
		VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
	};

	CheckExtensions(extensions, enumerateDeviceExtensionProperties());

	std::vector<RQueue::Family> queueFamilies;

	auto queueFamilyProperties = getQueueFamilyProperties();

	auto supportsPresent = false;
	for (uint32 i = 0; const auto& qFP : queueFamilyProperties) {

		RQueue::Family queueFamily{};
		queueFamily.index = i;
		queueFamily.props = qFP;
		queueFamily.supportsPresent = getSurfaceSupportKHR(i, surface);

		supportsPresent = queueFamily.supportsPresent ? true : supportsPresent;

		queueFamilies.emplace_back(queueFamily);

		i++;
	}

	CLOG_ABORT(!supportsPresent, "Present not supported by device");

	for (auto& fam : queueFamilies) {

		auto supportsGraphics = bool(fam.props.queueFlags & vk::QueueFlagBits::eGraphics);
		auto supportsTransfer = bool(fam.props.queueFlags & vk::QueueFlagBits::eTransfer);
		auto supportsCompute = bool(fam.props.queueFlags & vk::QueueFlagBits::eCompute);

		// graphics queue, must support graphics
		if (supportsGraphics) {
			graphicsFamily = fam;
		}

		// dma queue, must support transfer and not graphics/compute/present
		if (supportsTransfer && !supportsGraphics && !supportsCompute && !fam.supportsPresent) {
			dmaFamily = fam;
		}

		// compute queue, deticated compute (no graphics)
		if (supportsCompute && !supportsGraphics) {
			computeFamily = fam;
		}

		// present queue, CHECK: benchmark (queue count, graphics vs compute, etc)
		if (fam.supportsPresent && supportsGraphics) {
			presentFamily = fam;
		}
	}
}
} // namespace vl
