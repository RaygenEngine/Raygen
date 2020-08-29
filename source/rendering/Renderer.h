#pragma once
#include "engine/Listener.h"
#include "rendering/Device.h"
#include "rendering/ppt/PtCollection.h"
#include "rendering/scene/Scene.h"
#include "rendering/structures/GBuffer.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"
#include "rendering/output/OutputPassBase.h"
#include "rendering/passes/RaytracingPass.h"

namespace vl {

// TODO: tidy
inline class Renderer_ : public Listener {
	// The recommended framebuffer allocation size for the viewport.
	vk::Extent2D m_viewportFramebufferSize{};

private:
	void RecordGeometryPasses(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc);
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
	InFlightResources<GBuffer> m_gbuffer; // WIP: make this private when finished with the attachment descriptor set


	InFlightResources<RenderingPassInstance> m_ptPass;

	// TODO: RT, move those, framearray
	InFlightResources<vk::DescriptorSet> m_rtDescSet;
	InFlightResources<vk::DescriptorSet> m_rasterLightDescSet;

	//

	RaytracingPass m_raytracingPass;


	void DrawFrame(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, OutputPassBase& outputPass);

	void InitPipelines();


} * Renderer{};
} // namespace vl
