#pragma once

#include "engine/Listener.h"
#include "rendering/passes/offline/IrradianceMapCalculation.h"
#include "rendering/passes/offline/PathtracedCubemap.h"
#include "rendering/passes/offline/PrefilteredMapCalculation.h"
#include "rendering/passes/AOPass.h"
#include "rendering/passes/MirrorPass.h"
#include "rendering/ppt/techniques/PtLightBlend.h"
#include "rendering/wrappers/CmdBuffer.h"

namespace vl {
class OutputPassBase;
}
struct SceneRenderDesc;

namespace vl {

inline class Renderer_ : public Listener {

public:
	void InitPipelines();

	void ResizeBuffers(uint32 width, uint32 height);

	void DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc, OutputPassBase& outputPass);

	InFlightResources<vk::ImageView> GetOutputViews() const;

	// TODO: private
	InFlightResources<RenderingPassInstance> m_gbufferInst;
	InFlightResources<RenderingPassInstance> m_directLightPass;
	InFlightResources<RenderingPassInstance> m_indirectLightPass;
	InFlightResources<RenderingPassInstance> m_ptPass;

private:
	void RecordGeometryPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordRelfprobeEnvmapPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordRasterDirectPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordPostProcessPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);


	struct SecondaryBufferPool {

		std::vector<InFlightCmdBuffers<Graphics>> sBuffers;

		vk::CommandBuffer Get(uint32 frameIndex)
		{
			if (currBuffer > (int32(sBuffers.size()) - 1)) {
				sBuffers.emplace_back(InFlightCmdBuffers<Graphics>(vk::CommandBufferLevel::eSecondary));
			}

			return sBuffers[currBuffer++][frameIndex];
		}

		void Top() { currBuffer = 0; }

		int32 currBuffer{ 0 };

	} m_secondaryBuffersPool;

	vk::Extent2D m_extent{};

	// Global descriptor set
	InFlightResources<vk::DescriptorSet> m_attachmentsDesc;

	// TODO: tidy
	PtLightBlend m_lightblendPass;
	// MirrorPass m_mirrorPass;
	// AOPass m_aoPass;

	PathtracedCubemap m_ptCube;


	// PtCollection m_postprocCollection;

} * Renderer{};
} // namespace vl
