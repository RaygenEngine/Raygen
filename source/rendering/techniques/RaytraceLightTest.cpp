#include "RaytraceLightTest.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/pipes/TestSptPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneIrragrid.h"

// TODO: use specific for each technique instance and waitIdle() resize
ConsoleVariable<float> cons_testSptScale{ "r.lighttest.scale", 1.f, "Set lighttest scale" };

namespace vl {
RaytraceLightTest::RaytraceLightTest()
{
	imagesDescSet = Layouts->tripleStorageImage.AllocDescriptorSet();
	DEBUG_NAME(imagesDescSet, "progressive arealights storage image desc set");

	svgfPass.MakeLayout();
	svgfPass.MakePipeline();
}

ConsoleVariable<float> cons_rtSptScale{ "r.rtSpt.scale", 1.f, "Set scale of the pathtraced image." };
ConsoleVariable<int32> cons_rtSptBounces{ "r.rtSpt.bounces", 0, "Set the number of bounces of the pathtracer." };
ConsoleVariable<int32> cons_rtSptSamples{ "r.rtSpt.samples", 1, "Set the number of samples of the pathtracer." };

void RaytraceLightTest::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	// CHECK: this should not run if there are no area lights in the scene

	StaticPipes::Get<TestSptPipe>().Draw(
		cmdBuffer, sceneDesc, imagesDescSet, progressive.extent, frame++, cons_rtSptSamples, cons_rtSptBounces);

	svgfPass.SvgfDraw(cmdBuffer, sceneDesc, svgfRenderPassInstance);
}

void RaytraceLightTest::Resize(vk::Extent2D extent)
{
	progressive = RImage2D("ArealightProg",
		vk::Extent2D{ static_cast<uint32>(extent.width * cons_testSptScale),
			static_cast<uint32>(extent.height * cons_testSptScale) },
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

ConsoleVariable<int32> cons_svgfIters{ "r.svgf.iterations", 4, "Controls how many times to apply svgf atrous filter." };

ConsoleVariable<int32> cons_svgfProgressiveFeedback{ "r.svgf.feedbackIndex", -1,
	"Selects the index of the iteration to write onto the accumulation result (or do -1 to skip feedback)." };

ConsoleVariable<bool> cons_svgfEnable{ "r.svgf.enable", true, "Enable or disable svgf pass." };

void RaytraceLightTest::PtSvgf::SvgfDraw(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, RenderingPassInstance& rpInstance)
{
	auto times = *cons_svgfIters;
	for (int32 i = 0; i < std::max(times, 1); ++i) {
		rpInstance.RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u, &sceneDesc.globalDesc, 0u, nullptr);


			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 1u, 1u, &descriptorSets[i % 2], 0u, nullptr);

			SvgfPC pc{
				.iteration = i,
				.totalIter = (times < 1 || !cons_svgfEnable) ? 0 : times,
				.progressiveFeedbackIndex = cons_svgfProgressiveFeedback,
			};

			cmdBuffer.pushConstants(
				m_pipelineLayout.get(), vk::ShaderStageFlagBits::eFragment, 0u, sizeof(SvgfPC), &pc);
			cmdBuffer.draw(3u, 1u, 0u, 0u);
		});
	}
}

void RaytraceLightTest::PtSvgf::OnResize(vk::Extent2D extent, RaytraceLightTest& rtPass)
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

void RaytraceLightTest::PtSvgf::MakeLayout()
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

void RaytraceLightTest::PtSvgf::MakePipeline()
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