#include "pch.h"
#include "Device.h"

#include "rendering/VulkanLoader.h"

#include <set>

namespace vl {
Device_::Device_(RPhysicalDevice* pd, std::vector<const char*> deviceExtensions)
	: pd(pd)
{
	auto getPreferredFamily = [](auto families, auto queueCount) -> QueueFamily {
		for (auto& fam : families) {
			if (fam.props.queueCount == queueCount) {
				return fam;
			}
		}

		// TODO:
		LOG_ABORT("Adjust queue count to match your GPU queue families");
	};

	QueueFamily mainQueueFamily = getPreferredFamily(pd->graphicsFamilies, 16);
	QueueFamily dmaQueueFamily = getPreferredFamily(pd->transferFamilies, 2);
	QueueFamily computeQueueFamily = getPreferredFamily(pd->computeFamilies, 8);
	QueueFamily presentQueueFamily = getPreferredFamily(pd->presentFamilies, 16);

	// Get device's presentation queue
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32> uniqueQueueFamilies = {
		mainQueueFamily.index,
		dmaQueueFamily.index,
		computeQueueFamily.index,
		presentQueueFamily.index,
	};

	float qp1 = 1.0f;

	vk::DeviceQueueCreateInfo createInfo{};

	for (uint32 queueFamily : uniqueQueueFamilies) {
		vk::DeviceQueueCreateInfo createInfo{};
		createInfo.setQueueFamilyIndex(queueFamily)
			.setQueueCount(1u) //
			.setPQueuePriorities(&qp1);
		queueCreateInfos.push_back(createInfo);
	}


	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceBufferDeviceAddressFeatures,
		vk::PhysicalDeviceRayTracingFeaturesKHR>
		pDeviceFeaturesChain;
	auto& deviceFeatures = pDeviceFeaturesChain.get<vk::PhysicalDeviceFeatures2>();
	auto& deviceBufferAddressFeatures = pDeviceFeaturesChain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>();
	auto& deviceRayTracingFeatures = pDeviceFeaturesChain.get<vk::PhysicalDeviceRayTracingFeaturesKHR>();

	deviceFeatures.features.setSamplerAnisotropy(VK_TRUE);
	deviceBufferAddressFeatures.setBufferDeviceAddress(VK_TRUE);
	deviceRayTracingFeatures
		.setRayTracing(VK_TRUE) //
		.setRayQuery(VK_TRUE)
		.setRayTracingPrimitiveCulling(VK_TRUE)
		.setRayTracingIndirectTraceRays(VK_TRUE); // .setRayTracingHostAccelerationStructureCommands(VK_TRUE);

	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data())
		.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
		//.setPEnabledFeatures(&deviceFeatures)
		.setPpEnabledExtensionNames(deviceExtensions.data())
		.setEnabledExtensionCount(static_cast<uint32>(deviceExtensions.size()))
		.setEnabledLayerCount(0)
		.setPNext(&deviceFeatures);

	vk::Device::operator=(pd->createDevice(deviceCreateInfo));
	VulkanLoader::InitLoaderWithDevice(*this);

	// Device queues
	mainQueue.familyIndex = mainQueueFamily.index;
	mainQueue.SetHandle(getQueue(mainQueueFamily.index, 0));

	dmaQueue.familyIndex = dmaQueueFamily.index;
	dmaQueue.SetHandle(getQueue(dmaQueueFamily.index, 0));

	computeQueue.familyIndex = computeQueueFamily.index;
	computeQueue.SetHandle(getQueue(computeQueueFamily.index, 0));

	presentQueue.familyIndex = presentQueueFamily.index;
	presentQueue.SetHandle(getQueue(presentQueueFamily.index, 0));


	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.setQueueFamilyIndex(mainQueue.familyIndex);
	poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	mainCmdPool = createCommandPoolUnique(poolInfo);

	poolInfo.setQueueFamilyIndex(dmaQueue.familyIndex);
	poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	dmaCmdPool = createCommandPoolUnique(poolInfo);

	poolInfo.setQueueFamilyIndex(computeQueue.familyIndex);
	poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	computeCmdPool = createCommandPoolUnique(poolInfo);

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary).setCommandPool(dmaCmdPool.get()).setCommandBufferCount(1u);

	dmaCmdBuffer = allocateCommandBuffers(allocInfo)[0];

	allocInfo.setCommandPool(mainCmdPool.get());

	mainCmdBuffer = allocateCommandBuffers(allocInfo)[0];

	allocInfo.setCommandPool(computeCmdPool.get());

	computeCmdBuffer = allocateCommandBuffers(allocInfo)[0];
}

uint32 Device_::FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties) const
{
	vk::PhysicalDeviceMemoryProperties memProperties = pd->getMemoryProperties();

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
		vk::FormatProperties props = pd->getFormatProperties(format);

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

SwapchainSupportDetails Device_::GetSwapchainSupportDetails() const
{
	SwapchainSupportDetails ssDetails;

	ssDetails.capabilities = pd->getSurfaceCapabilitiesKHR(pd->surface);
	ssDetails.formats = pd->getSurfaceFormatsKHR(pd->surface);
	ssDetails.presentModes = pd->getSurfacePresentModesKHR(pd->surface);
	return ssDetails;
}

Device_::~Device_()
{
	dmaCmdPool.reset();
	mainCmdPool.reset();

	destroy();
}
} // namespace vl
