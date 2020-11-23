#pragma once

#include "engine/Listener.h"
#include "rendering/passes/bake/ComputeCubemapArrayConvolution.h"
#include "rendering/passes/bake/ComputeCubemapConvolution.h"
#include "rendering/passes/bake/ComputePrefilteredConvolution.h"
#include "rendering/passes/bake/PathtracedCubemap.h"
#include "rendering/passes/bake/PathtracedCubemapArray.h"
#include "rendering/passes/gi/IndirectSpecularPass.h"
#include "rendering/passes/gi/MirrorPass.h"
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
	InFlightResources<RenderingPassInstance> m_mainPassInst;
	InFlightResources<RenderingPassInstance> m_secondaryPassInst;
	InFlightResources<RenderingPassInstance> m_ptPass;
	IndirectSpecularPass m_indirectSpecPass;
	MirrorPass m_mirorPass;

private:
	void RecordMapPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordMainPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordSecondaryPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordPostProcessPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);

	vk::Extent2D m_extent{};

	// Global descriptor set
	InFlightResources<vk::DescriptorSet> m_attachmentsDesc;

	// TODO: tidy
	PtLightBlend m_lightblendPass;
	PathtracedCubemap m_ptCubemap;
	PathtracedCubemapArray m_ptCubemapArray;
	ComputeCubemapArrayConvolution m_compCubemapArrayConvolution;
	ComputeCubemapConvolution m_compCubemapConvolution;
	ComputePrefilteredConvolution m_compPrefilteredConvolution;

	// PtCollection m_postprocCollection;

} * Renderer{};
} // namespace vl
