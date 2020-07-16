#pragma once

namespace vk {
class Instance;
class Device;
} // namespace vk


struct VulkanLoader {
	static void InitLoaderBase();
	static void InitLoaderWithInstance(vk::Instance& instance);
	static void InitLoaderWithDevice(vk::Device& device);
};
