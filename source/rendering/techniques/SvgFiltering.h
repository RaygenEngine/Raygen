#pragma once

#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

struct SceneRenderDesc;

namespace vl {
struct SvgFiltering {
	SvgFiltering();

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

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, float minColorAlpha,
		float minMomentsAlpha, int32 totalIterations, float phiColor, float phiNormal, bool luminanceMode);
	void AttachInputImage(const RImage2D& inputImage);
	vk::ImageView GetFilteredImageView(size_t frameIndex) const
	{
		return svgfRenderPassInstance.at(frameIndex).framebuffer["SvgfFinalModulated"].view();
	}
};

} // namespace vl
