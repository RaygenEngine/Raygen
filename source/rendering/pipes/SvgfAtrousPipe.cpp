#include "SvgfAtrousPipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"

namespace {
struct PushConstant {
	int32 iteration;
	int32 totalIter;
	float phiColor;
	float phiNormal;
	bool luminanceMode;
};
static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniquePipelineLayout SvgfAtrousPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>(
		{
			DescriptorLayouts->global.handle(),
			DescriptorLayouts->_3storageImage.handle(),
		},
		vk::ShaderStageFlagBits::eFragment);
}

vk::UniquePipeline SvgfAtrousPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/svgf/atrous.shader");
	gpuShader.onCompile = [&]() {
		MakePipeline();
	};

	return rvk::makeGraphicsPipeline(gpuShader.shaderStages, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, layout(), PassLayouts->svgf.compatibleRenderPass.get(), 0u);
}

void SvgfAtrousPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc,
	vk::DescriptorSet inputOutputStorageImages, int32 iteration, int32 totalIterations, float phiColor, float phiNormal,
	bool luminanceMode) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 0u,
		{
			sceneDesc.globalDesc,
			inputOutputStorageImages,
		},
		nullptr);

	PushConstant pc{
		.iteration = iteration,
		.totalIter = totalIterations,
		.phiColor = phiColor,
		.phiNormal = phiNormal,
		.luminanceMode = luminanceMode,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eFragment, 0u, sizeof(PushConstant), &pc);

	// big triangle
	cmdBuffer.draw(3u, 1u, 0u, 0u);
}

} // namespace vl
