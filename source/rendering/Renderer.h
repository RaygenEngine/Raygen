#pragma once

#include "engine/Listener.h"
#include "rendering/ppt/techniques/PtLightBlend.h"
#include "rendering/techniques/CalculateIrragrids.h"
#include "rendering/techniques/CalculateReflprobes.h"
#include "rendering/techniques/CalculateShadowmaps.h"
#include "rendering/techniques/RaytraceMirrorReflections.h"
#include "rendering/wrappers/CmdBuffer.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

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

	InFlightResources<RenderingPassInstance> m_mainPassInst;
	InFlightResources<RenderingPassInstance> m_secondaryPassInst;

	// TODO: ppt
	PtLightBlend m_ptLightBlend;
	InFlightResources<RenderingPassInstance> m_ptPass;
	//
private:
	void RecordMapPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordMainPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordSecondaryPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordPostProcessPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);

	vk::Extent2D m_extent{};

	// Global descriptor set
	InFlightResources<vk::DescriptorSet> m_attachmentsDesc;

	// techniques
	CalculateShadowmaps m_calculateShadowmaps;
	CalculateReflprobes m_calculateReflprobes;
	CalculateIrragrids m_calculateIrragrids;
	RaytraceMirrorReflections m_raytraceMirrorReflections;


} * Renderer{};
} // namespace vl
