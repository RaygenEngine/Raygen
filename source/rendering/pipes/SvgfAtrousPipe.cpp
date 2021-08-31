#include "SvgfAtrousPipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"

namespace {
struct PushConstant {
	int32 iteration;
	int32 totalIter;
	int32 progressiveFeedbackIndex;
	float phiColor;
	float phiNormal;
};
static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniquePipelineLayout SvgfAtrousPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>(
		{
			DescriptorLayouts->global.handle(),
			DescriptorLayouts->_3storageImage.handle(),
		},
		vk::ShaderStageFlagBits::eFragment);
}

vk::UniquePipeline SvgfAtrousPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/svgf/atrous.shader");
	gpuShader.onCompile = [&]() {
		MakePipeline();
	};

	return rvk::makeGraphicsPipeline(gpuShader.shaderStages, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, layout(), PassLayouts->svgf.compatibleRenderPass.get(), 0u);
}

void SvgfAtrousPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc,
	vk::DescriptorSet inputOutputStorageImages, int32 iteration, int32 totalIter) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);


	static ConsoleVariable<bool> cons_enable{ "r.svgf.enable", true, "Enable or disable svgf pass." };

	static ConsoleVariable<int32> cons_progressiveFeedback{ "r.svgf.feedbackIndex", -1,
		"Selects the index of the iteration to write onto the accumulation result (or do -1 to skip feedback)." };

	static ConsoleVariable<float> cons_phiColor{ "r.svgf.phiColor", 1.f, "Set atrous filter phiColor." };

	static ConsoleVariable<float> cons_phiNormal{ "r.svgf.phiNormal", 0.2f, "Set atrous filter phiNormal." };


	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 0u,
		{
			sceneDesc.globalDesc,
			inputOutputStorageImages,
		},
		nullptr);

	PushConstant pc{
		.iteration = iteration,
		.totalIter = (totalIter < 1 || !cons_enable) ? 0 : totalIter,
		.progressiveFeedbackIndex = cons_progressiveFeedback,
		.phiColor = *cons_phiColor,
		.phiNormal = *cons_phiNormal,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eFragment, 0u, sizeof(PushConstant), &pc);

	// big triangle
	cmdBuffer.draw(3u, 1u, 0u, 0u);
}

} // namespace vl
