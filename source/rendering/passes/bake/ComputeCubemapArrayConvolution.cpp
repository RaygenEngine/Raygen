#include "ComputeCubemapArrayConvolution.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/scene/SceneIrradianceGrid.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/wrappers/ImageView.h"

namespace {
struct PushConstant {
	int32 samples;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
ComputeCubemapArrayConvolution::ComputeCubemapArrayConvolution()
{
	MakeCompPipeline();
}

void ComputeCubemapArrayConvolution::RecordPass(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneIrradianceGrid& ig)
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline.get());

	PushConstant pc{
		ig.ptSamples, // WIP: convolution samples here
	};

	cmdBuffer.pushConstants(m_pipelineLayout.get(), vk::ShaderStageFlagBits::eCompute, 0u, sizeof(PushConstant), &pc);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, m_pipelineLayout.get(), 0u, 1u, &ig.irradianceStorageDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_pipelineLayout.get(), 1u, 1u,
		&ig.uboDescSet[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, m_pipelineLayout.get(), 2u, 1u, &ig.environmentSamplerDescSet, 0u, nullptr);

	cmdBuffer.dispatch(ig.resolution / 32, ig.resolution / 32, 1);
}

void ComputeCubemapArrayConvolution::MakeCompPipeline()
{
	std::array layouts{
		Layouts->cubemapArrayStorage.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->cubemapArray.handle(),
	};

	// all rt shaders here
	GpuAsset<Shader>& shader = GpuAssetManager->CompileShader("engine-data/spv/compute/irradiance-array.shader");
	shader.onCompileRayTracing = [&]() {
		MakeCompPipeline();
	};

	std::vector<vk::PipelineShaderStageCreateInfo> stages;

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
