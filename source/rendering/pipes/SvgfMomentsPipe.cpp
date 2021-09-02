#include "SvgfMomentsPipe.h"

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
	bool luminanceMode;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {

vk::UniquePipelineLayout SvgfMomentsPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>(
		{
			DescriptorLayouts->global.handle(),
			DescriptorLayouts->_1imageSampler_3storageImage.handle(),
		},
		vk::ShaderStageFlagBits::eCompute);
}

vk::UniquePipeline SvgfMomentsPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/svgf/moments.shader");
	gpuShader.onCompileRayTracing = [&]() {
		StaticPipes::Recompile<SvgfMomentsPipe>();
	};


	vk::ComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStage(gpuShader.compute.Lock().shaderStageCreateInfo) //
		.setLayout(layout())
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return Device->createComputePipelineUnique(nullptr, pipelineInfo);
}

void SvgfMomentsPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent,
	const SceneRenderDesc& sceneDesc, vk::DescriptorSet inputOutputsImageDescSet, float minColorAlpha,
	float minMomentsAlpha, bool luminanceMode) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, layout(), 0u,
		{
			sceneDesc.globalDesc,
			inputOutputsImageDescSet,
		},
		nullptr);

	PushConstant pc{
		minColorAlpha,
		minMomentsAlpha,
		luminanceMode,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eCompute, 0u, sizeof(PushConstant), &pc);

	cmdBuffer.dispatch(extent.width / 32, extent.height / 32, 1); // TODO: see AccumulationPipe example
}

} // namespace vl
