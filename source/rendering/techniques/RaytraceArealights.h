#pragma once
#include "rendering/ppt/PtBase.h"
#include "rendering/wrappers/ImageView.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"
#include "rendering/techniques/SvgFiltering.h"

struct SceneRenderDesc;

namespace vl {
struct RaytraceArealights {
	RaytraceArealights();

	RImage2D pathtracedResult;
	vk::DescriptorSet pathtracingInputDescSet;

	SvgFiltering svgFiltering;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 samples, float minColorAlpha,
		float minMomentsAlpha, int32 totalIterations, float phiColor, float phiNormal);
	void Resize(vk::Extent2D extent);
};

} // namespace vl
