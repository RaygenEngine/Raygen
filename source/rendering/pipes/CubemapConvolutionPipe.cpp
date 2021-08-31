#include "CubemapConvolutionPipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/Device.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneReflProbe.h"

namespace {
struct PushConstant {
	glm::mat4 viewInv;
	glm::mat4 projInv;
};

static_assert(sizeof(PushConstant) <= 128);
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

	return rvk::makeComputePipeline(gpuShader.compute.Lock().shaderStageCreateInfo, layout());
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
		viewInv,
		projInv,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eCompute, 0u, sizeof(PushConstant), &pc);

	cmdBuffer.dispatch(extent.width / 32, extent.height / 32, 1); // TODO: see AccumulationPipe example
}

} // namespace vl
