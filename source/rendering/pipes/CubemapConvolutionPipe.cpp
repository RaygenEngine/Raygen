#include "CubemapConvolutionPipe.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/core/PipeUtl.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/wrappers/ImageView.h"

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
	return rvk::makePipelineLayoutEx(
		{
			Layouts->singleStorageImage.handle(),
			Layouts->singleSamplerDescLayout.handle(),
		},
		vk::ShaderStageFlagBits::eCompute, sizeof(PushConstant));
}

vk::UniquePipeline CubemapConvolutionPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/compute/irradiance.shader");
	gpuShader.onCompileRayTracing = [&]() {
		StaticPipes::Recompile<CubemapConvolutionPipe>();
	};

	vk::ComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStage(gpuShader.compute.Lock().shaderStageCreateInfo) //
		.setLayout(layout())
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return Device->createComputePipelineUnique(nullptr, pipelineInfo);
}

void CubemapConvolutionPipe::Draw(vk::CommandBuffer cmdBuffer, vk::DescriptorSet storageImageDescSet,
	vk::DescriptorSet environmentSamplerDescSet, const vk::Extent3D& extent, const glm::mat4& viewInv,
	const glm::mat4& projInv) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, layout(), 0u, 1u, &storageImageDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, layout(), 1u, 1u, &environmentSamplerDescSet, 0u, nullptr);

	PushConstant pc{
		viewInv,
		projInv,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eCompute, 0u, sizeof(PushConstant), &pc);

	cmdBuffer.dispatch(extent.width / 32, extent.height / 32, 1);
}

} // namespace vl
