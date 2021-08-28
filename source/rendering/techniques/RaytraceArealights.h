#pragma once
#include "rendering/ppt/PtBase.h"
#include "rendering/wrappers/ImageView.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

struct SceneRenderDesc;

namespace vl {
struct RaytraceArealights {

	RaytraceArealights();

	RImage2D progressive;
	RImage2D momentsBuffer;

	RenderingPassInstance svgfRenderPassInstance;

	vk::DescriptorSet imagesDescSet;

	int32 frame{ 0 };

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void Resize(vk::Extent2D extent);

public:
	class PtSvgf : public PtBase_SinglePipeline {
	public:
		void MakeLayout() override;
		void MakePipeline() override;

		// By default rt should write directly in Svgf's Img 0 for performance
		void SvgfDraw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, RenderingPassInstance& rpInstance);


		// DescSet 0:
		//    Img 0 is read  (index: 2)
		//    Img 1 is write (index: 3)
		//
		// DescSet 1:
		//    Img 1 is read  (index: 2)
		//    Img 0 is write (index: 3)
		std::array<vk::DescriptorSet, 2> descriptorSets;

		std::array<RImage2D, 2> swappingImages;


		void OnResize(vk::Extent2D extent, RaytraceArealights& rtPass);


		void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) override{};
	};


	PtSvgf svgfPass;
};

} // namespace vl
