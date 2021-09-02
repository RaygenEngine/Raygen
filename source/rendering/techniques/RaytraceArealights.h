#pragma once
#include "rendering/ppt/PtBase.h"
#include "rendering/wrappers/ImageView.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

struct SceneRenderDesc;

namespace vl {
struct RaytraceArealights {
	RaytraceArealights();

	RImage2D pathtracedResult;
	vk::DescriptorSet pathtracingInputDescSet;

	RImage2D progressive;

	RImage2D momentsHistory;
	vk::DescriptorSet inputOutputsDescSet;
	InFlightResources<RenderingPassInstance> svgfRenderPassInstance;

	// DescSet 0:
	//    Img 0 is read  (index: 2)
	//    Img 1 is write (index: 3)
	//
	// DescSet 1:
	//    Img 1 is read  (index: 2)
	//    Img 0 is write (index: 3)
	std::array<vk::DescriptorSet, 2> descriptorSets;
	std::array<RImage2D, 2> swappingImages;

	int32 iteration{ 0 };

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 samples, float minColorAlpha,
		float minMomentsAlpha, int32 totalIterations, float phiColor, float phiNormal);
	void Resize(vk::Extent2D extent);
};

} // namespace vl
