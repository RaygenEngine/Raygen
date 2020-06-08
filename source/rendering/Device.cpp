#include "pch.h"
#include "Device.h"

#include <set>

namespace vl {
QueueFamily GetQueueFamilyWithBestRating(const std::vector<QueueFamily>& queueFamilies)
{
	auto it = std::max_element(
		queueFamilies.begin(), queueFamilies.end(), [](QueueFamily a, QueueFamily b) { return a.rating < b.rating; });

	return { *it };
}

Device_::Device_(RPhysicalDevice* pd, std::vector<const char*> deviceExtensions)
	: pd(pd)
{
	auto graphicsQueueFamily = GetQueueFamilyWithBestRating(pd->graphicsFamilies);
	auto transferQueueFamily = GetQueueFamilyWithBestRating(pd->transferFamilies);
	auto presentQueueFamily = GetQueueFamilyWithBestRating(pd->presentFamilies);

	// Get device's presentation queue
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32> uniqueQueueFamilies
		= { graphicsQueueFamily.index, transferQueueFamily.index, presentQueueFamily.index };

	float qp1 = 1.0f;

	vk::DeviceQueueCreateInfo createInfo{};

	for (uint32 queueFamily : uniqueQueueFamilies) {
		vk::DeviceQueueCreateInfo createInfo{};
		createInfo.setQueueFamilyIndex(queueFamily)
			.setQueueCount(1u) //
			.setPQueuePriorities(&qp1);
		queueCreateInfos.push_back(createInfo);
	}


	// NEXT: check if supported by the pd..
	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.setSamplerAnisotropy(VK_TRUE);

	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data())
		.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
		.setPEnabledFeatures(&deviceFeatures)
		.setPpEnabledExtensionNames(deviceExtensions.data())
		.setEnabledExtensionCount(static_cast<uint32>(deviceExtensions.size()))
		.setEnabledLayerCount(0);

	vk::Device::operator=(pd->createDevice(deviceCreateInfo));

	// Device queues
	graphicsQueue.familyIndex = graphicsQueueFamily.index;
	graphicsQueue.SetHandle(getQueue(graphicsQueueFamily.index, 0));

	transferQueue.familyIndex = transferQueueFamily.index;
	transferQueue.SetHandle(getQueue(transferQueueFamily.index, 0));

	presentQueue.familyIndex = presentQueueFamily.index;
	presentQueue.SetHandle(getQueue(presentQueueFamily.index, 0));


	vk::CommandPoolCreateInfo graphicsPoolInfo{};
	graphicsPoolInfo.setQueueFamilyIndex(graphicsQueue.familyIndex);
	graphicsPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	graphicsCmdPool = createCommandPoolUnique(graphicsPoolInfo);

	vk::CommandPoolCreateInfo transferPoolInfo{};
	transferPoolInfo.setQueueFamilyIndex(transferQueue.familyIndex);
	transferPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	transferCmdPool = createCommandPoolUnique(transferPoolInfo);

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandPool(transferCmdPool.get())
		.setCommandBufferCount(1u);

	transferCmdBuffer = allocateCommandBuffers(allocInfo)[0];

	allocInfo.setCommandPool(graphicsCmdPool.get());

	graphicsCmdBuffer = allocateCommandBuffers(allocInfo)[0];
}

Device_::~Device_()
{
	transferCmdPool.reset();
	graphicsCmdPool.reset();

	destroy();
}
} // namespace vl
