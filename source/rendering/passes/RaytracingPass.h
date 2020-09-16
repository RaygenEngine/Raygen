#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/Image.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"
#include "rendering/ppt/PtBase.h"

struct SceneRenderDesc;

namespace vl {
class Renderer_;

class RaytracingPass {
public:
	vk::UniquePipeline m_rtPipeline;
	vk::UniquePipelineLayout m_rtPipelineLayout;
	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;

	RImage2D m_progressiveResult;
	RImage2D m_momentsBuffer;


	InFlightResources<RImageAttachment> m_indirectResult;


	int32 m_rtFrame{ 0 };

	void MakeRtPipeline();

	void CreateRtShaderBindingTable();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, Renderer_* renderer);

	void Resize(vk::Extent2D extent);

	InFlightResources<RenderingPassInstance> m_svgfRenderPassInstance;


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


		void OnResize(vk::Extent2D extent, RaytracingPass& rtPass);


		void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) override{};
	};

	PtSvgf svgfPass;
};

} // namespace vl
