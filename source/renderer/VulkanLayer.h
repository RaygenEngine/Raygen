#pragma once
#include "engine/Events.h"
#include "platform/GlfwUtl.h"
#include "renderer/DeferredPass.h"
#include "renderer/Device.h"
#include "renderer/EditorPass.h"
#include "renderer/GeometryPass.h"
#include "renderer/Instance.h"
#include "renderer/Model.h"
#include "renderer/PhysicalDevice.h"
#include "renderer/Swapchain.h"

#include <vulkan/vulkan.hpp>

struct UniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
};

inline class VulkanLayer : public Object {

public:
	//
	// Framebuffer size, Window size, recommended framebuffer size and more
	//

	// What is:
	// Viewport Framebuffer Size: (recommended framebuffer size)
	// The size we actually want our framebuffers to allocate.
	// This usually should be larger than the actual viewport size to allow seamless resizing (dynamic state) of the
	// viewport while in the editor. In the real game this would always match the swapchain size and not waste any
	// memory.
	// The algorithm works like this:
	//   if: x <= 1920 & y <= 1080 -> framebufferSize == 1920x1080
	// elif: x <= 2560 & y <= 1440 -> framebufferSize == 2560x1440
	// elif: x <= 4096 & y <= 2160 -> framebufferSize == 4096x2160
	// else: framebufferSize == viewport


	// The recommended framebuffer allocation size for the viewport.
	vk::Extent2D viewportFramebufferSize{};

	// The actual game viewport rectangle in swapchain coords
	vk::Rect2D viewportRect{};


protected:
	// CHECK: boolflag event, (impossible to use here current because of init order)
	BoolFlag didViewportResize;
	BoolFlag didWindowResize;

	void OnViewportResize();
	void OnWindowResize();
	void UpdateQuadDescriptorSet();


public:
	//
	//
	//
	VulkanLayer(std::vector<const char*>& extensions, GLFWwindow* window);
	~VulkanLayer();


	void Init();
	void ReconstructSwapchain();

	Instance* instance;
	std::unique_ptr<Swapchain> swapchain;


	// Model descriptors
	//
	vk::UniqueDescriptorSetLayout modelDescriptorSetLayout;
	vk::UniqueDescriptorPool modelDescriptorPool;
	std::vector<std::unique_ptr<Model>> models;
	vk::UniqueBuffer uniformBuffers;
	vk::UniqueDeviceMemory uniformBuffersMemory;
	//

	// Quad descriptors
	vk::UniqueDescriptorSetLayout quadDescriptorSetLayout;
	vk::UniqueDescriptorPool quadDescriptorPool;
	vk::UniqueDescriptorSet quadDescriptorSet;
	vk::UniqueSampler quadSampler;

	//

	vk::CommandBuffer geometryCmdBuffer;

	std::vector<vk::CommandBuffer> outCmdBuffer;

	// sync objects
	vk::UniqueSemaphore imageAvailableSemaphore;
	vk::UniqueSemaphore renderFinishedSemaphore;

	GeometryPass geomPass;
	DeferredPass defPass;
	EditorPass editorPass;


	void ReinitModels();

	void InitModelDescriptors();
	vk::DescriptorSet GetModelDescriptorSet();

	void InitQuadDescriptor();
	void DrawFrame();

} * Layer;
