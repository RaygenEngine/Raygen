#include "CubemapConvolutionPipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneReflProbe.h"

namespace {
struct PushConstant {
	glm::mat4 viewInv;
	glm::mat4 projInv;
	int32 width;
	int32 height;
};

// TODO:
// static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniquePipelineLayout CubemapConvolutionPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>(
		{
			DescriptorLayouts->_1storageImage.handle(),
			DescriptorLayouts->_1imageSampler.handle(),
		},
		vk::ShaderStageFlagBits::eCompute);
}

vk::UniquePipeline CubemapConvolutionPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/compute/irradiance.shader");
	gpuShader.onCompileRayTracing = [&]() {
		StaticPipes::Recompile<CubemapConvolutionPipe>();
	};

	auto shaderStageCreateInfo = gpuShader.compute.Lock().shaderStageCreateInfo;

	return rvk::makeComputePipeline(shaderStageCreateInfo, layout());
}

void CubemapConvolutionPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent,
	vk::DescriptorSet storageImageDescSet, vk::DescriptorSet environmentSamplerDescSet, const glm::mat4& viewInv,
	const glm::mat4& projInv) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, layout(), 0u,
		{
			storageImageDescSet,
			environmentSamplerDescSet,
		},
		nullptr);

	PushConstant pc{
		.viewInv = viewInv,
		.projInv = projInv,
		.width = static_cast<int32>(extent.width),
		.height = static_cast<int32>(extent.height),
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eCompute, 0u, sizeof(PushConstant), &pc);

	uint32 groupCountX = ((extent.width) / 32u) + 1u;
	uint32 groupCountY = ((extent.height) / 32u) + 1u;

	cmdBuffer.dispatch(groupCountX, groupCountY, 1u);
}

} // namespace vl
