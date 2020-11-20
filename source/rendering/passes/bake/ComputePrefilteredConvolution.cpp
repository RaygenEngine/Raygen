#include "ComputePrefilteredConvolution.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
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
ComputePrefilteredConvolution::ComputePrefilteredConvolution()
{
	MakeCompPipeline();
}

void ComputePrefilteredConvolution::RecordPass(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneReflprobe& rp)
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline.get());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, m_pipelineLayout.get(), 0u, 1u, &rp.prefilteredStorageDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_pipelineLayout.get(), 1u, 1u,
		&rp.uboDescSet[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, m_pipelineLayout.get(), 2u, 1u, &rp.environmentSamplerDescSet, 0u, nullptr);

	for (int mip = 0; mip < rp.ubo.lodCount; ++mip) {

		PushConstant pc{
			mip,
			rp.prefSamples,
		};

		cmdBuffer.pushConstants(
			m_pipelineLayout.get(), vk::ShaderStageFlagBits::eCompute, 0u, sizeof(PushConstant), &pc);

		uint32 mipResolution = static_cast<uint32>(rp.prefiltered.extent.width * std::pow(0.5, mip));

		cmdBuffer.dispatch(mipResolution / 32, mipResolution / 32, 1);
	}
}

void ComputePrefilteredConvolution::MakeCompPipeline()
{
	std::array layouts{
		Layouts->storageImageArray10.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->singleSamplerDescLayout.handle(),
	};

	// all rt shaders here
	GpuAsset<Shader>& shader = GpuAssetManager->CompileShader("engine-data/spv/compute/prefiltered.shader");
	shader.onCompileRayTracing = [&]() {
		MakeCompPipeline();
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

	m_pipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);

	vk::ComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStage(shader.compute.Lock().shaderStageCreateInfo) //
		.setLayout(m_pipelineLayout.get())
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	m_pipeline = Device->createComputePipelineUnique(nullptr, pipelineInfo);
}
} // namespace vl
