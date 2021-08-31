#include "AccumulationPipe.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneReflProbe.h"
#include "engine/Engine.h"

namespace {
struct PushConstant {
	int32 iteration;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {

ConsoleFunction<> cons_refitAccumulation{ "r.accumulation.recompile",
	[]() { StaticPipes::Recompile<AccumulationPipe>(); }, "Recompiles accumulation compute shader." };

vk::UniquePipelineLayout AccumulationPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>(
		{
			DescriptorLayouts->_2storageImage.handle(),
		},
		vk::ShaderStageFlagBits::eCompute);
}

vk::UniquePipeline AccumulationPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/compute/accumulation.shader");
	gpuShader.onCompileRayTracing = [&]() {
		StaticPipes::Recompile<AccumulationPipe>();
	};

	std::array<vk::SpecializationMapEntry, 2> specializationMapEntries;
	specializationMapEntries[0]
		.setConstantID(0) //
		.setSize(sizeof(specializationData.sizeX))
		.setOffset(0);
	specializationMapEntries[1]
		.setConstantID(1) //
		.setSize(sizeof(specializationData.sizeY))
		.setOffset(offsetof(SpecializationData, sizeY));

	vk::SpecializationInfo specializationInfo{};
	specializationInfo
		.setDataSize(sizeof(specializationData)) //
		.setMapEntryCount(static_cast<uint32>(specializationMapEntries.size()))
		.setPMapEntries(specializationMapEntries.data())
		.setPData(&specializationData);

	auto shaderStageCreateInfo = gpuShader.compute.Lock().shaderStageCreateInfo;
	shaderStageCreateInfo.setPSpecializationInfo(&specializationInfo);

	vk::ComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStage(shaderStageCreateInfo) //
		.setLayout(layout())
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	auto res = std::make_pair<uint32, uint32>(32u, 32u); // TODO: findBestValues(g_ViewportCoordinates.size.x,
														 // g_ViewportCoordinates.size.y); <-- leetcode required here

	specializationData.sizeX = res.first;
	specializationData.sizeY = res.second;
	return rvk::makeComputePipeline(shaderStageCreateInfo, layout());
}

void AccumulationPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent,
	vk::DescriptorSet inputOutputStorageImages, int32 iteration) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, layout(), 0u,
		{
			inputOutputStorageImages,
		},
		nullptr);

	PushConstant pc{
		iteration,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eCompute, 0u, sizeof(PushConstant), &pc);

	cmdBuffer.dispatch(extent.width / specializationData.sizeX, extent.height / specializationData.sizeY, 1);
}

} // namespace vl
