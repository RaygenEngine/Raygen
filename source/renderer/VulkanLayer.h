#pragma once

#include "renderer/Instance.h"
#include "renderer/PhysicalDevice.h"
#include "renderer/Swapchain.h"
#include "renderer/LogicalDevice.h"
#include "platform/GlfwUtil.h"
#include "renderer/Model.h"
#include "renderer/GeometryPass.h"
#include "renderer/DeferredPass.h"

#include <vulkan/vulkan.hpp>
struct UniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
};
// WIP:
class VulkanLayer {


public:
	static void InitVulkanLayer(std::vector<const char*>& extensions, WindowType* window);
	static void ReconstructSwapchain();

	inline static std::unique_ptr<Instance> instance;
	inline static std::unique_ptr<LogicalDevice> device;
	inline static std::unique_ptr<Swapchain> swapchain;

	// Model descriptors
	//
	inline static vk::UniqueDescriptorSetLayout modelDescriptorSetLayout;
	inline static vk::UniqueDescriptorPool modelDescriptorPool;
	inline static std::vector<std::unique_ptr<Model>> models;
	inline static vk::UniqueBuffer uniformBuffers;
	inline static vk::UniqueDeviceMemory uniformBuffersMemory;
	//

	// Quad descriptors
	inline static vk::UniqueDescriptorSetLayout quadDescriptorSetLayout;
	inline static vk::UniqueDescriptorPool quadDescriptorPool;
	inline static vk::UniqueDescriptorSet quadDescriptorSet;
	//

	// WIP: one of many
	inline static vk::UniqueCommandBuffer geometryCmdBuffer;

	inline static std::vector<vk::UniqueCommandBuffer> outCmdBuffer;

	// sync objects
	inline static vk::UniqueSemaphore imageAvailableSemaphore;
	inline static vk::UniqueSemaphore renderFinishedSemaphore;

	inline static GeometryPass geomPass;
	inline static DeferredPass defPass;

	static void ReinitModels();

	static void InitModelDescriptors();
	static vk::DescriptorSet GetModelDescriptorSet();

	static void InitQuadDescriptor();


	static void DrawFrame();
};
