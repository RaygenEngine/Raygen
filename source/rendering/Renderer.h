#pragma once

#include "engine/Listener.h"
#include "rendering/passes/AOPass.h"
#include "rendering/passes/IndirectSpecularPass.h"
#include "rendering/passes/offline/IrradianceMapCalculation.h"
#include "rendering/passes/offline/PathtracedCubemap.h"
#include "rendering/passes/offline/PrefilteredMapCalculation.h"
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
	IndirectSpecularPass m_indirectSpecPass;

private:
	void RecordGeometryPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordGiPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordDirectPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordIndirectPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordPostProcessPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);

	vk::Extent2D m_extent{};

	// Global descriptor set
	InFlightResources<vk::DescriptorSet> m_attachmentsDesc;

	// TODO: tidy
	PtLightBlend m_lightblendPass;

	// AOPass m_aoPass;

	PathtracedCubemap m_ptCube;


	// PtCollection m_postprocCollection;

} * Renderer{};
} // namespace vl
