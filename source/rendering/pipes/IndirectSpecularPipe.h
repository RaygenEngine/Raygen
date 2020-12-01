// TODO: when and if used look at MirrorPipe.h
//#pragma once
//#include "rendering/pipes/StaticPipeBase.h"
//#include "rendering/wrappers/Buffer.h"
//#include "rendering/wrappers/ImageView.h"
//
// namespace vl {
//
// class IndirectSpecularPipe : public StaticPipeBase {
//	vk::UniquePipelineLayout MakePipelineLayout() override;
//	vk::UniquePipeline MakePipeline() override;
//
//	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const;
//
//	void Resize(vk::Extent2D extent);
//
//	// TODO: techniques
//	InFlightResources<RenderingPassInstance> m_svgfRenderPassInstance;
//
//
//	vk::UniquePipeline m_rtPipeline;
//	vk::UniquePipelineLayout m_rtPipelineLayout;
//
//	RBuffer m_rtSBTBuffer;
//	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;
//
//	RImage2D m_progressiveResult;
//	RImage2D m_momentsBuffer;
//	InFlightResources<vk::DescriptorSet> m_rtDescSet;
//
// public:
//	class PtSvgf : public PtBase_SinglePipeline {
//	public:
//		void MakeLayout() override;
//		void MakePipeline() override;
//
//		// By default rt should write directly in Svgf's Img 0 for performance
//		void SvgfDraw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, RenderingPassInstance& rpInstance);
//
//
//		// DescSet 0:
//		//    Img 0 is read  (index: 2)
//		//    Img 1 is write (index: 3)
//		//
//		// DescSet 1:
//		//    Img 1 is read  (index: 2)
//		//    Img 0 is write (index: 3)
//		std::array<vk::DescriptorSet, 2> descriptorSets;
//
//		std::array<RImage2D, 2> swappingImages;
//
//
//		void OnResize(vk::Extent2D extent, IndirectSpecularPass& rtPass);
//
//
//		void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) override{};
//	};
//
//
//	PtSvgf svgfPass;
//};
//
//} // namespace vl
