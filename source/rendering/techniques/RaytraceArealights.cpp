#include "RaytraceArealights.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/Layouts.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/pipes/ArealightsPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneIrragrid.h"

// TODO: use specific for each technique instance and waitIdle() resize
ConsoleVariable<float> cons_arealightsScale{ "r.arealights.scale", 1.f, "Set arealights scale" };

namespace vl {
RaytraceArealights::RaytraceArealights()
{
	imagesDescSet = Layouts->tripleStorageImage.AllocDescriptorSet();
	DEBUG_NAME(imagesDescSet, "progressive arealights storage image desc set");

	svgfPass.MakeLayout();
	svgfPass.MakePipeline();
}

void RaytraceArealights::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	// CHECK: this should not run if there are no area lights in the scene

	StaticPipes::Get<ArealightsPipe>().Draw(cmdBuffer, sceneDesc, imagesDescSet, progressive.extent, frame++);

	svgfPass.SvgfDraw(cmdBuffer, sceneDesc, svgfRenderPassInstance);
}

void RaytraceArealights::Resize(vk::Extent2D extent)
{
	progressive = RImage2D("ArealightProg",
		vk::Extent2D{ static_cast<uint32>(extent.width * cons_arealightsScale),
			static_cast<uint32>(extent.height * cons_arealightsScale) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);


	momentsBuffer = RImage2D("Moments Buffer", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);

	svgfRenderPassInstance = Layouts->svgfPassLayout.CreatePassInstance(extent.width, extent.height);

	svgfPass.OnResize(extent, *this);


	rvk::writeDescriptorImages(imagesDescSet, 0u,
		{
			svgfPass.swappingImages[0].view(),
			progressive.view(),
			momentsBuffer.view(),
		},
		nullptr, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
}

struct SvgfPC {
	int32 iteration{ 0 };
	int32 totalIter{ 0 };
	int32 progressiveFeedbackIndex{ 0 };
};
static_assert(sizeof(SvgfPC) <= 128);

ConsoleVariable<int32> console_SvgfIters{ "rt.svgf.iterations", 4,
	"Controls how many times to apply svgf atrous filter." };

ConsoleVariable<int32> console_SvgfProgressiveFeedback{ "rt.svgf.feedbackIndex", -1,
	"Selects the index of the iteration to write onto the accumulation result (or do -1 to skip feedback)" };

ConsoleVariable<bool> console_SvgfEnable{ "rt.svgf.enable", true, "Enable or disable svgf pass." };

void RaytraceArealights::PtSvgf::SvgfDraw(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, RenderingPassInstance& rpInstance)
{
	auto times = *console_SvgfIters;
	for (int32 i = 0; i < std::max(times, 1); ++i) {
		rpInstance.RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u, &sceneDesc.globalDesc, 0u, nullptr);


			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 1u, 1u, &descriptorSets[i % 2], 0u, nullptr);

			SvgfPC pc{
				.iteration = i,
				.totalIter = (times < 1 || !console_SvgfEnable) ? 0 : times,
				.progressiveFeedbackIndex = console_SvgfProgressiveFeedback,
			};

			cmdBuffer.pushConstants(
				m_pipelineLayout.get(), vk::ShaderStageFlagBits::eFragment, 0u, sizeof(SvgfPC), &pc);
			cmdBuffer.draw(3u, 1u, 0u, 0u);
		});
	}
}

void RaytraceArealights::PtSvgf::OnResize(vk::Extent2D extent, RaytraceArealights& rtPass)
{
	swappingImages[0] = RImage2D("SVGF 0", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);

	swappingImages[1] = RImage2D("SVGF 1", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);

	for (size_t j = 0; j < 2; ++j) {
		descriptorSets[j] = Layouts->quadStorageImage.AllocDescriptorSet();

		rvk::writeDescriptorImages(descriptorSets[j], 0u,
			{
				rtPass.progressive.view(),
				rtPass.momentsBuffer.view(),
				swappingImages[(j + 0) % 2].view(),
				swappingImages[(j + 1) % 2].view(),
			},
			nullptr, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
	}
}

void RaytraceArealights::PtSvgf::MakeLayout()
{
	std::array layouts{
		Layouts->globalDescLayout.handle(),
		Layouts->quadStorageImage.handle(),
	};

	vk::PushConstantRange pcRange;
	pcRange
		.setSize(sizeof(SvgfPC)) //
		.setStageFlags(vk::ShaderStageFlagBits::eFragment)
		.setOffset(0);

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayouts(layouts) //
		.setPushConstantRanges(pcRange);

	m_pipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

void RaytraceArealights::PtSvgf::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/raytrace/test/svgf.shader");
	gpuShader.onCompile = [&]() {
		MakePipeline();
	};

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
						   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) //
		.setBlendEnable(VK_FALSE);

	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending
		.setLogicOpEnable(VK_FALSE) //
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachments(colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });


	Utl_CreatePipelineCustomPass(gpuShader, colorBlending, *Layouts->svgfPassLayout.compatibleRenderPass);
}

} // namespace vl
