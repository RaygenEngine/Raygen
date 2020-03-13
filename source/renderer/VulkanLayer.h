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

	bool isMinimzed{ false };

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

	std::vector<UniquePtr<Scene_Model>> models;


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
