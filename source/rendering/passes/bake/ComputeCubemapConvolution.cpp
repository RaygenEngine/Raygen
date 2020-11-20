#include "ComputeCubemapConvolution.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/wrappers/ImageView.h"


namespace vl {
ComputeCubemapConvolution::ComputeCubemapConvolution()
{
	MakeCompPipeline();
}

void ComputeCubemapConvolution::RecordPass(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneReflprobe& rp)
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline.get());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, m_pipelineLayout.get(), 0u, 1u, &rp.irradianceStorageDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, m_pipelineLayout.get(), 1u, 1u, &rp.environmentSamplerDescSet, 0u, nullptr);

	cmdBuffer.dispatch(rp.irrResolution / 32, rp.irrResolution / 32, 1);
}

void ComputeCubemapConvolution::MakeCompPipeline()
{
	std::array layouts{
		Layouts->singleStorageImage.handle(),
		Layouts->singleSamplerDescLayout.handle(),
	};

	// all rt shaders here
	GpuAsset<Shader>& shader = GpuAssetManager->CompileShader("engine-data/spv/compute/irradiance.shader");
	shader.onCompileRayTracing = [&]() {
		MakeCompPipeline();
	};

	std::vector<vk::PipelineShaderStageCreateInfo> stages;


	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setSetLayouts(layouts);

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
