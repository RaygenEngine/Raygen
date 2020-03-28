#pragma once
#include "engine/Events.h"
#include "platform/GlfwUtl.h"
#include "rendering/asset/Model.h"
#include "rendering/DeferredPass.h"
#include "rendering/EditorPass.h"
#include "rendering/GeometryPass.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/wrapper/Buffer.h"
#include "rendering/wrapper/PhysicalDevice.h"
#include "rendering/wrapper/Swapchain.h"
#include "rendering/scene/SceneStructs.h"
#include "rendering/scene/Scene.h"


#include <vulkan/vulkan.hpp>


namespace vl {

struct UBO_Globals {
	glm::mat4 viewProj;
};
using SemVec = std::vector<vk::Semaphore>;

inline class Renderer_ : public Listener {
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

	bool isMinimzed{ false };

public:
	Renderer_();
	~Renderer_();


	void Init();
	void ReconstructSwapchain();

	GeometryPass geomPass;
	DeferredPass defPass;
	EditorPass editorPass;


	Swapchain* swapchain{};

	// NEXT:
	R_DescriptorLayout debugDescSetLayout;
	void InitDebugDescriptors();

	//
	std::vector<vk::CommandBuffer> geometryCmdBuffer;
	std::vector<vk::CommandBuffer> outCmdBuffer;


	std::vector<vk::UniqueFence> inFlightFence;

	std::vector<vk::UniqueSemaphore> renderFinishedSem;
	std::vector<vk::UniqueSemaphore> imageAvailSem;

	uint32 currentFrame{ 0 };

	void DrawGeometryPass(vk::CommandBuffer cmdBuffer);

	void UpdateForFrame();

	void DrawDeferredPass(vk::CommandBuffer cmdBuffer, vk::Framebuffer framebuffer);


	void DrawFrame();
} * Renderer{};
} // namespace vl
