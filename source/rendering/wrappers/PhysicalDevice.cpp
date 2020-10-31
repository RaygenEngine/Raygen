#include "PhysicalDevice.h"


namespace vl {
RPhysicalDevice::RPhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR inSurface)
	: vk::PhysicalDevice(vkHandle)
	, surface(inSurface)
{
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

	auto propertiesChain = getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPropertiesKHR>();
	genProps = propertiesChain.get<vk::PhysicalDeviceProperties2>();
	rtProps = propertiesChain.get<vk::PhysicalDeviceRayTracingPropertiesKHR>();

	auto featuresChain = getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceBufferDeviceAddressFeatures,
		vk::PhysicalDeviceRayTracingFeaturesKHR>();
	genFeats = featuresChain.get<vk::PhysicalDeviceFeatures2>();
	bufferFeats = featuresChain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>();
	rtFeats = featuresChain.get<vk::PhysicalDeviceRayTracingFeaturesKHR>();

	auto rtSupport = rtFeats.rayTracing && rtFeats.rayQuery && rtProps.shaderGroupHandleSize == 32u
					 && rtProps.maxRecursionDepth >= 1 && bufferFeats.bufferDeviceAddress;

	CLOG_ABORT(!rtSupport || !supportsPresent, "Rendering Device not supported!");

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
