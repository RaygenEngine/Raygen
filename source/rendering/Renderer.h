#pragma once
#include "engine/Listener.h"
#include "rendering/Device.h"
#include "rendering/ppt/PtCollection.h"
#include "rendering/scene/Scene.h"
#include "rendering/structures/GBuffer.h"

namespace vl {

// TODO: tidy
inline class Renderer_ : public Listener {
	// The recommended framebuffer allocation size for the viewport.
	vk::Extent2D m_viewportFramebufferSize{};

	// The actual game viewport rectangle in m_swapchain coords
	vk::Rect2D m_viewportRect{};


private:
	// cpyhdrtexture TODO: tidy
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	InFlightResources<GBuffer> m_gbuffer;


	void RecordGeometryPasses(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordRayTracingPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordPostProcessPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordOutPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc, vk::RenderPass outRp,
		vk::Framebuffer outFb, vk::Extent2D outExtent);

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
	BoolFlag m_didViewportResize;

	void OnViewportResize();

public:
	InFlightResources<RFramebuffer> m_ptPassFramebuffer;

	// std::array<UniquePtr<RImageAttachment>, 3> m_attachmentsDepthToUnlit;

	InFlightResources<vk::DescriptorSet> m_ppDescSet;


	InFlightResources<vk::DescriptorSet> m_wipDescSet;

	// TODO: RT, move those, framearray
	vk::UniqueAccelerationStructureKHR m_sceneAS;
	InFlightResources<vk::DescriptorSet> m_rtDescSet;

	vk::UniquePipeline m_rtPipeline;
	vk::UniquePipelineLayout m_rtPipelineLayout;
	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;

	int32 m_rtFrame{ 0 };
	int32 m_rtDepth{ 3 };
	int32 m_rtSamples{ 2 };

	void MakeRtPipeline();
	void SetRtImage();
	void CreateRtShaderBindingTable();


	Renderer_();

	void PrepareForFrame();

	void DrawFrame(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc, vk::RenderPass outRp,
		vk::Framebuffer outFb, vk::Extent2D outExtent);

	void InitPipelines(vk::RenderPass outRp);
	void MakeCopyHdrPipeline(vk::RenderPass outRp);


} * Renderer{};
} // namespace vl
