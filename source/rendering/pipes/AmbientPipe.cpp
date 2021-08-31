#include "AmbientPipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/VkCoreIncludes.h"

namespace {
struct PushConstant {
	float bias;
	float strength;
	float radius;
	int32 samples;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniquePipelineLayout AmbientPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>(
		{
			DescriptorLayouts->global.handle(),
			DescriptorLayouts->accelerationStructure.handle(),
		},
		vk::ShaderStageFlagBits::eFragment);
}

vk::UniquePipeline AmbientPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/lighting/ambient.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<AmbientPipe>();
	};

	return rvk::makeGraphicsPipeline(gpuShader.shaderStages, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, layout(), PassLayouts->secondary.compatibleRenderPass.get(), 0u);
}

void AmbientPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 0u,
		{
			sceneDesc.globalDesc,
			sceneDesc.scene->sceneAsDescSet,
		},
		nullptr);

	static ConsoleVariable<float> cons_bias{ "r.ao.bias", 0.001f, "Set the ambient occlusion bias." };
	static ConsoleVariable<float> cons_strength{ "r.ao.strength", 1.0f, "Set the ambient occlusion strength." };
	static ConsoleVariable<float> cons_radius{ "r.ao.radius", .2f, "Set the ambient occlusion radius." };
	static ConsoleVariable<int32> cons_samples{ "r.ao.samples", 4, "Set the ambient occlusion samples." };

	PushConstant pc{
		cons_bias,
		cons_strength,
		cons_radius,
		cons_samples,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eFragment, 0u, sizeof(PushConstant), &pc);

	// big triangle
	cmdBuffer.draw(3u, 1u, 0u, 0u);
}

} // namespace vl
