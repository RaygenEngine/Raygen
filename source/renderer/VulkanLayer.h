#pragma once
#include "engine/Events.h"
#include "platform/GlfwUtl.h"
#include "renderer/asset/Model.h"
#include "renderer/DeferredPass.h"
#include "renderer/EditorPass.h"
#include "renderer/GeometryPass.h"
#include "renderer/PoolAllocator.h"
#include "renderer/wrapper/Buffer.h"
#include "renderer/wrapper/Instance.h"
#include "renderer/wrapper/PhysicalDevice.h"
#include "renderer/wrapper/Swapchain.h"

#include <vulkan/vulkan.hpp>

struct UBO_Globals {
	glm::mat4 viewProj;
};
using SemVec = std::vector<vk::Semaphore>;

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
	PoolAllocator poolAllocator;

	//
	//
	//
	VulkanLayer(std::vector<const char*>& extensions, GLFWwindow* window);
	~VulkanLayer();


	void Init();
	void ReconstructSwapchain();

	Instance* instance;
	UniquePtr<Swapchain> swapchain;

	// Global descriptors
	R_DescriptorLayout globalUboDescLayout;
	vk::DescriptorSet globalUboDescSet;

	UniquePtr<Buffer> globalsUbo;

	std::vector<std::unique_ptr<Scene_Model>> models;


	// Quad descriptors

	R_DescriptorLayout quadDescLayout;

	vk::DescriptorSet quadDescSet;
	vk::UniqueSampler quadSampler;


	R_DescriptorLayout debugDescSetLayout;

	//
	vk::CommandBuffer geometryCmdBuffer;

	std::vector<vk::CommandBuffer> outCmdBuffer;

	// sync objects


	vk::UniqueSemaphore swapchainImageReadySem;
	vk::UniqueSemaphore gbufferReadySem;

	vk::UniqueSemaphore renderFinishedSemaphore;

	vk::UniqueSemaphore imageAcquiredSem;

	GeometryPass geomPass;
	DeferredPass defPass;
	EditorPass editorPass;


	void DrawGeometryPass(
		std::vector<vk::PipelineStageFlags> waitStages, SemVec waitSemaphores, SemVec signalSemaphores);
	void UpdateForFrame();
	void DrawDeferredPass(std::vector<vk::PipelineStageFlags> waitStages, SemVec waitSemaphores,
		SemVec signalSemaphores, vk::CommandBuffer cmdBuffer, vk::Framebuffer framebuffer);


	void ReinitModels();

	void InitModelDescriptors();
	void InitDebugDescriptors();

	vk::UniqueSampler GetDefaultSampler();

	void InitQuadDescriptor();
	void DrawFrame();

} * Layer;
