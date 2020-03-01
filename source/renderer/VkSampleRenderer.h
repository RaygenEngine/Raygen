//#pragma once
//#include "renderer/Instance.h"
//#include "renderer/LogicalDevice.h"
//#include "renderer/Swapchain.h"
//#include "renderer/Model.h"
//
//#include "system/EngineEvents.h"
//
//#include <vulkan/vulkan.hpp>
//
//
// class VkSampleRenderer {
//	friend class ImguiImpl;
//
//	// final step renderpass and framebuffers
//	//
//	vk::UniqueRenderPass m_renderPass;
//	std::vector<vk::UniqueFramebuffer> m_framebuffers;
//
//	// graphics pipeline stuff
//	//
//	vk::UniqueDescriptorSetLayout m_descriptorSetLayout;
//	vk::UniquePipeline m_pipeline;
//	vk::UniquePipelineLayout m_pipelineLayout;
//
//	// global uniforms
//	//
//	std::vector<vk::UniqueBuffer> m_uniformBuffers;
//	std::vector<vk::UniqueDeviceMemory> m_uniformBuffersMemory;
//
//	// descriptor pool / sets
//
//	// data
//	//
//	std::vector<std::unique_ptr<Model>> m_models;
//
//	// render commands
//	//
//	std::vector<vk::UniqueCommandBuffer> m_renderCmdBuffers;
//
//	// sync objects
//	vk::UniqueSemaphore m_imageAvailableSemaphore;
//	vk::UniqueSemaphore m_renderFinishedSemaphore;
//
//	void CreateGeometry();
//
//	// WIP : handle resizing explicitly in case there are drivers
//	// that do not report swap chain incompatibilities correctly
//	// Also test window minimization
//	// void AllocateRenderCommandBuffers();
//
//	DECLARE_EVENT_LISTENER(m_resizeListener, Event::OnWindowResize);
//	DECLARE_EVENT_LISTENER(m_worldLoaded, Event::OnWorldLoaded);
//	DECLARE_EVENT_LISTENER(m_viewportUpdated, Event::OnViewportUpdated);
//	bool m_shouldRecreateSwapchain{ false };
//	bool m_shouldRecreatePipeline{ false };
//
//
//	void RecordCommandBuffer(int32 imageIndex);
//
//	void InitRenderPassAndFramebuffersOfCurrentSwapchain();
//	void InitGraphicsPipeline();
//	void InitRenderCmdBuffers();
//	void InitUniformBuffers();
//	void InitWorld();
//
// public:
//	virtual ~VkSampleRenderer();
//
//	void Init();
//
//	void DrawFrame();
//};
