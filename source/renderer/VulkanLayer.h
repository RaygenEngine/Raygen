#pragma once

#include "renderer/Instance.h"
#include "renderer/PhysicalDevice.h"
#include "renderer/Swapchain.h"
#include "renderer/LogicalDevice.h"
#include "core/Event.h"
#include "platform/GlfwUtil.h"
#include "renderer/Model.h"
#include "renderer/GeometryPass.h"
#include "renderer/DeferredPass.h"
#include "renderer/EditorPass.h"
#include "engine/EngineEvents.h"

#include <vulkan/vulkan.hpp>
struct UniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
};

class VulkanLayer {

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
	inline static vk::Extent2D viewportFramebufferSize{};

	// The actual game viewport rectangle in swapchain coords
	inline static vk::Rect2D viewportRect{};


protected:
	// CHECK: boolflag event, (impossible to use here current because of init order)
	inline static BoolFlag didViewportResize;
	inline static BoolFlag didWindowResize;

	inline static DECLARE_EVENT_LISTENER(viewportUpdateListener, Event::OnViewportUpdated);
	inline static DECLARE_EVENT_LISTENER(windowResizeListener, Event::OnWindowResize);

	static void OnViewportResize();
	static void OnWindowResize();
	static void UpdateQuadDescriptorSet();

public:
	//
	//
	//
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
	inline static vk::UniqueSampler quadSampler;

	//

	inline static vk::UniqueCommandBuffer geometryCmdBuffer;

	inline static std::vector<vk::UniqueCommandBuffer> outCmdBuffer;

	// sync objects
	inline static vk::UniqueSemaphore imageAvailableSemaphore;
	inline static vk::UniqueSemaphore renderFinishedSemaphore;

	inline static GeometryPass geomPass;
	inline static DeferredPass defPass;
	inline static EditorPass editorPass;


	static void ReinitModels();

	static void InitModelDescriptors();
	static vk::DescriptorSet GetModelDescriptorSet();

	static void InitQuadDescriptor();


	static void DrawFrame();
};
