#include "CubemapArrayConvolutionPipe.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneIrragrid.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/wrappers/ImageView.h"

namespace {
struct PushConstant {
	int32 x;
	int32 y;
	int32 z;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniquePipelineLayout CubemapArrayConvolutionPipe::MakePipelineLayout()
{
	std::array layouts{
		Layouts->cubemapArrayStorage.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->cubemapArray.handle(),
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

vk::UniquePipeline CubemapArrayConvolutionPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/compute/irradiance-array.shader");
	gpuShader.onCompileRayTracing = [&]() {
		StaticPipes::Recompile<CubemapArrayConvolutionPipe>();
	};

	vk::ComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStage(gpuShader.compute.Lock().shaderStageCreateInfo) //
		.setLayout(layout())
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return Device->createComputePipelineUnique(nullptr, pipelineInfo);
}

void CubemapArrayConvolutionPipe::Draw(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneIrragrid& ig) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, layout(), 0u, 1u, &ig.irradianceStorageDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, layout(), 1u, 1u, &ig.uboDescSet[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute, layout(), 2u, 1u, &ig.environmentSamplerDescSet, 0u, nullptr);

	for (int32 x = 0; x < ig.ubo.width; ++x) {
		for (int32 y = 0; y < ig.ubo.height; ++y) {
			for (int32 z = 0; z < ig.ubo.depth; ++z) {

				PushConstant pc{
					x,
					y,
					z,
				};

				cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eCompute, 0u, sizeof(PushConstant), &pc);


				cmdBuffer.dispatch(ig.irrResolution / 32, ig.irrResolution / 32, 1);
			}
		}
	}
}
} // namespace vl
