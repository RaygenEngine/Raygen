#pragma once
#include "engine/Listener.h"
#include "rendering/Device.h"
#include "rendering/output/OutputPassBase.h"
#include "rendering/passes/RaytracingPass.h"
#include "rendering/ppt/PtCollection.h"
#include "rendering/ppt/techniques/PtLightBlend.h"
#include "rendering/scene/Scene.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {

// TODO: tidy
inline class Renderer_ : public Listener {

private:
	void RecordGeometryPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordRasterDirectPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordPostProcessPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);

	// PtCollection m_postprocCollection;

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
	vk::Extent2D m_extent{};

	void ResizeBuffers(uint32 width, uint32 height);
	InFlightResources<vk::ImageView> GetOutputViews() const;

	InFlightResources<RenderingPassInstance> m_gbufferInst;
	InFlightResources<RenderingPassInstance> m_rasterDirectPass;

	InFlightResources<RenderingPassInstance> m_ptPass;

	// Global descriptor set
	InFlightResources<vk::DescriptorSet> m_attachmentsDesc;


	// TODO: RT, move those
	InFlightResources<vk::DescriptorSet> m_rtDescSet;

	PtLightBlend lightblendPass;

	//

	RaytracingPass m_raytracingPass;


	void DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc, OutputPassBase& outputPass);

	void InitPipelines();


} * Renderer{};
} // namespace vl
