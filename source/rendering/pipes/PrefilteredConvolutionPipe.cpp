#include "PrefilteredConvolutionPipe.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/wrappers/ImageView.h"

namespace {
struct PushConstant {
	int32 mip;
	int32 samples;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {

vk::UniquePipelineLayout PrefilteredConvolutionPipe::MakePipelineLayout()
{
	std::array layouts{
		Layouts->storageImageArray10.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->singleSamplerDescLayout.handle(),
	};

	// pipeline layout
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eCompute) //
		.setSize(sizeof(PushConstant))
		.setOffset(0u);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setPushConstantRanges(pushConstantRange) //
		.setSetLayouts(layouts);

	return Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

vk::UniquePipeline PrefilteredConvolutionPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/compute/prefiltered.shader");
	gpuShader.onCompileRayTracing = [&]() {
		StaticPipes::Recompile<PrefilteredConvolutionPipe>();
	};


	vk::ComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStage(gpuShader.compute.Lock().shaderStageCreateInfo) //
		.setLayout(layout())
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return Device->createComputePipelineUnique(nullptr, pipelineInfo);
}

void PrefilteredConvolutionPipe::Draw(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneReflprobe& rp) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, layout(), 0u, 1u, &rp.prefilteredStorageDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, layout(), 1u, 1u, &rp.uboDescSet[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, layout(), 2u, 1u, &rp.environmentSamplerDescSet, 0u, nullptr);

	for (int mip = 0; mip < rp.ubo.lodCount; ++mip) {

		PushConstant pc{
			mip,
			rp.prefSamples,
		};

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eCompute, 0u, sizeof(PushConstant), &pc);

		uint32 mipResolution = static_cast<uint32>(rp.prefiltered.extent.width * std::pow(0.5, mip));

		cmdBuffer.dispatch(mipResolution / 32, mipResolution / 32, 1);
	}
}

} // namespace vl
