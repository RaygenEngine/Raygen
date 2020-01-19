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
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkDevice m_device;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	VkSurfaceKHR m_surface;


	VkSwapchainKHR m_swapChain;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;

	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;

	void CreateInstance();
	void CreateSurface(HWND assochWnd, HINSTANCE instance);
	void CreateDevice(VkPhysicalDevice physicalDevice, const QueueFamilyIndices& indices);
	void CreateSwapChain(VkPhysicalDevice physicalDevice, const QueueFamilyIndices& indices);
	void CreateSwapChainImageViews();

public:
	virtual ~VkRendererBase();


	virtual void Init(HWND assochWnd, HINSTANCE instance) override;
	virtual bool SupportsEditor() override;
	virtual void Render() override;
};

} // namespace vk
