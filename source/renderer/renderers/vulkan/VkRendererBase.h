#pragma once

#include "renderer/ObserverRenderer.h"

#include <vulkan/vulkan.h>

namespace vk {

struct QueueFamilyIndices {
	std::optional<uint32> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};


struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VkRendererBase : public ObserverRenderer {

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			return attributeDescriptions;
		}
	};

	const std::vector<Vertex> m_vertices = { { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } }, { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } } };


	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkPhysicalDevice m_physicalDevice{ VK_NULL_HANDLE };

	VkDevice m_device;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	// TODO: transfer queue

	VkSurfaceKHR m_surface;

	VkSwapchainKHR m_swapChain;
	// WIP: during swap chain recreation destroy as soon as you are finished using it
	VkSwapchainKHR m_oldSwapChain;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;

	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;

	VkRenderPass m_renderPass;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_graphicsPipeline;

	std::vector<VkFramebuffer> m_swapChainFramebuffers;

	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;

	VkSemaphore m_imageAvailableSemaphore;
	VkSemaphore m_renderFinishedSemaphore;

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;

	// WIP : handle resizing explicitly in case there are drivers
	// that do not report swap chain incompatibilities correctly
	// Also test window minimization

	void CreateInstance();
	void CreateSurface(HWND assochWnd, HINSTANCE instance);
	void ChoosePhysicalDevice();
	void CreateDevice();
	void CreateSwapChain();
	void CreateSwapChainImageViews();
	void CreateRenderPasses();
	void CreateGraphicsPipeline();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateVertexBuffer();
	void CreateCommandBuffers();
	void CreateSemaphores();

	void RecreateSwapChain();
	void CleanupSwapChain();

public:
	virtual ~VkRendererBase();


	virtual void Init(HWND assochWnd, HINSTANCE instance) override;
	virtual bool SupportsEditor() override;
	virtual void Render() override;
};

} // namespace vk
