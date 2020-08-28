#pragma once
#include "engine/Listener.h"
#include "rendering/Device.h"
#include "rendering/ppt/PtCollection.h"
#include "rendering/scene/Scene.h"
#include "rendering/structures/GBuffer.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"
#include "rendering/output\OutputPassBase.h"

namespace vl {

// TODO: tidy
inline class Renderer_ : public Listener {
	// The recommended framebuffer allocation size for the viewport.
	vk::Extent2D m_viewportFramebufferSize{};

	// The actual game viewport rectangle in m_swapchain coords
	vk::Rect2D m_viewportRect{};


private:
	InFlightResources<GBuffer> m_gbuffer;


	void RecordGeometryPasses(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordRayTracingPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordPostProcessPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc);

	PtCollection m_postprocCollection;

	struct SecondaryBufferPool {

		std::vector<InFlightResources<vk::CommandBuffer>> sBuffers;

		vk::CommandBuffer Get(uint32 frameIndex)
		{
			if (currBuffer > (int32(sBuffers.size()) - 1)) {

				vk::CommandBufferAllocateInfo allocInfo{};
				allocInfo.setCommandPool(Device->graphicsCmdPool.get())
					.setLevel(vk::CommandBufferLevel::eSecondary)
					.setCommandBufferCount(c_framesInFlight);

				sBuffers.emplace_back(Device->allocateCommandBuffers(allocInfo));
			}

			return sBuffers[currBuffer++][frameIndex];
		}

		void Top() { currBuffer = 0; }

		int32 currBuffer{ 0 };

	} m_secondaryBuffersPool;


protected:
	// CHECK: boolflag event, (impossible to use here current because of init order)


public:
	void ResizeBuffers(uint32 width, uint32 height);
	InFlightResources<vk::ImageView> GetOutputViews() const;

	InFlightResources<RenderingPassInstance> m_ptPass;

	// TODO: RT, move those, framearray
	InFlightResources<vk::DescriptorSet> m_wipDescSet;

	vk::UniqueAccelerationStructureKHR m_sceneAS;
	InFlightResources<vk::DescriptorSet> m_rtDescSet;

	vk::UniquePipeline m_rtPipeline;
	vk::UniquePipelineLayout m_rtPipelineLayout;
	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;

	int32 m_rtFrame{ 0 };
	int32 m_rtDepth{ 2 };
	int32 m_rtSamples{ 1 };

	void MakeRtPipeline();
	void SetRtImage();
	void CreateRtShaderBindingTable();
	//


	void DrawFrame(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, OutputPassBase& outputPass);

	void InitPipelines();


} * Renderer{};
} // namespace vl
