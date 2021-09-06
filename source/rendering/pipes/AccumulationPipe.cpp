#include "AccumulationPipe.h"

#include "engine/Engine.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneReflProbe.h"
#include "engine/Events.h"

namespace {
struct PushConstant {
	int32 iteration;
	int32 width;
	int32 height;
};

// TODO:
// static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {

vk::UniquePipelineLayout AccumulationPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>(
		{
			DescriptorLayouts->_2storageImage.handle(),
		},
		vk::ShaderStageFlagBits::eCompute);
}

vk::UniquePipeline AccumulationPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/compute/accumulation.shader");
	gpuShader.onCompileRayTracing = [&]() {
		StaticPipes::Recompile<AccumulationPipe>();
	};

	auto shaderStageCreateInfo = gpuShader.compute.Lock().shaderStageCreateInfo;


	return rvk::makeComputePipeline(shaderStageCreateInfo, layout());
}

void AccumulationPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent,
	vk::DescriptorSet inputOutputStorageImages, int32 iteration) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, layout(), 0u,
		{
			inputOutputStorageImages,
		},
		nullptr);


	PushConstant pc{
		.iteration = iteration,
		.width = static_cast<int32>(extent.width),
		.height = static_cast<int32>(extent.height),
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eCompute, 0u, sizeof(PushConstant), &pc);

	uint32 groupCountX = ((extent.width) / 32u) + 1u;
	uint32 groupCountY = ((extent.height) / 32u) + 1u;

	cmdBuffer.dispatch(groupCountX, groupCountY, 1u);
}

} // namespace vl
