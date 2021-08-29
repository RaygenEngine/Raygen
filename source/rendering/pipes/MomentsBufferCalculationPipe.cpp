#include "MomentsBufferCalculationPipe.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/Layouts.h"

namespace {
struct PushConstant {
	float minColorAlpha;
	float minMomentsAlpha;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {

vk::UniquePipelineLayout MomentsBufferCalculationPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayoutEx(
		{
			DescriptorLayouts->global.handle(),
			DescriptorLayouts->_1imageSampler_2storageImage.handle(),
		},
		vk::ShaderStageFlagBits::eCompute, sizeof(PushConstant));
}

vk::UniquePipeline MomentsBufferCalculationPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/compute/moments.shader");
	gpuShader.onCompileRayTracing = [&]() {
		StaticPipes::Recompile<MomentsBufferCalculationPipe>();
	};


	vk::ComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStage(gpuShader.compute.Lock().shaderStageCreateInfo) //
		.setLayout(layout())
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return Device->createComputePipelineUnique(nullptr, pipelineInfo);
}

void MomentsBufferCalculationPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent,
	vk::DescriptorSet inputOutputsImageDescSet, const SceneRenderDesc& sceneDesc) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, layout(), 0u,
		{
			sceneDesc.globalDesc,
			inputOutputsImageDescSet,
		},
		nullptr);

	static ConsoleVariable<float> cons_minColorAlpha{ "r.svgf.minColorAlpha", 0.05f,
		"Set SVGF color alpha for reprojection mix." };
	static ConsoleVariable<float> cons_minMomentsAlpha{ "r.svgf.minMomentsAlpha", 0.05f,
		"Set SVGF moments alpha for reprojection mix." };

	PushConstant pc{
		*cons_minColorAlpha,
		*cons_minMomentsAlpha,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eCompute, 0u, sizeof(PushConstant), &pc);

	cmdBuffer.dispatch(extent.width / 32, extent.height / 32, 1); // TODO: see AccumulationPipe example
}

} // namespace vl
