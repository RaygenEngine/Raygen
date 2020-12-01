#include "CubemapConvolutionPipe.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/core/PipeUtl.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/wrappers/ImageView.h"


namespace vl {
vk::UniquePipelineLayout CubemapConvolutionPipe::MakePipelineLayout()
{
	rvk::makeLayoutNoPC({
		Layouts->singleStorageImage.handle(),
		Layouts->singleSamplerDescLayout.handle(),
	});
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

void CubemapConvolutionPipe::Draw(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneReflprobe& rp) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, layout(), 0u, 1u, &rp.irradianceStorageDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, layout(), 1u, 1u, &rp.environmentSamplerDescSet, 0u, nullptr);

	cmdBuffer.dispatch(rp.irrResolution / 32, rp.irrResolution / 32, 1);
}

} // namespace vl
