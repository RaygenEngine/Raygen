#include "SvgfModulatePipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Layouts.h"
#include "rendering/scene/Scene.h"

namespace vl {
vk::UniquePipelineLayout SvgfModulatePipe::MakePipelineLayout()
{
	return rvk::makePipelineLayoutNoPC({
		DescriptorLayouts->global.handle(),
		PassLayouts->svgf.internalDescLayout.handle(),
	});
}

vk::UniquePipeline SvgfModulatePipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/svgf/modulate.shader");
	gpuShader.onCompile = [&]() {
		MakePipeline();
	};

	return rvk::makeGraphicsPipeline(gpuShader.shaderStages, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, layout(), PassLayouts->svgf.compatibleRenderPass.get(), 1u);
}

void SvgfModulatePipe::RecordCmd(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet inputDescSet) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 0u,
		{
			sceneDesc.globalDesc,
			inputDescSet,
		},
		nullptr);


	// big triangle
	cmdBuffer.draw(3u, 1u, 0u, 0u);
}

} // namespace vl
