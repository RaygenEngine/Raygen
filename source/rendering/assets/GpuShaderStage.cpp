#include "GpuShaderStage.h"

#include "assets/AssetRegistry.h"
#include "assets/pods/ShaderStage.h"
#include "assets/util/SpirvCompiler.h"
#include "rendering/VkCoreIncludes.h"
#include "rendering/Device.h"

using namespace vl;

namespace {
vk::ShaderStageFlagBits StageToVulkan(ShaderStageType stage)
{
	switch (stage) {
		case ShaderStageType::Vertex: return vk::ShaderStageFlagBits::eVertex;
		case ShaderStageType::Fragment: return vk::ShaderStageFlagBits::eFragment;
		case ShaderStageType::RayGen: return vk::ShaderStageFlagBits::eRaygenKHR;
		case ShaderStageType::Intersect: return vk::ShaderStageFlagBits::eIntersectionKHR;
		case ShaderStageType::AnyHit: return vk::ShaderStageFlagBits::eAnyHitKHR;
		case ShaderStageType::ClosestHit: return vk::ShaderStageFlagBits::eClosestHitKHR;
		case ShaderStageType::Miss: return vk::ShaderStageFlagBits::eMissKHR;
		case ShaderStageType::Geometry: return vk::ShaderStageFlagBits::eGeometry;
		case ShaderStageType::TessControl: return vk::ShaderStageFlagBits::eTessellationControl;
		case ShaderStageType::TessEvaluation: return vk::ShaderStageFlagBits::eTessellationEvaluation;
		case ShaderStageType::Callable: return vk::ShaderStageFlagBits::eCallableKHR;
		case ShaderStageType::Compute: return vk::ShaderStageFlagBits::eCompute;
	}

	LOG_ABORT("StageToVulkan failed to find stage.");
}
} // namespace

GpuShaderStage::GpuShaderStage(PodHandle<ShaderStage> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

bool GpuShaderStage::HasValidModule() const
{
	return bool(module);
}

bool GpuShaderStage::WasLastCompileSuccessful() const
{
	return lastCompileSuccess;
}

void GpuShaderStage::Update(const AssetUpdateInfo& info)
{
	auto podPtr = podHandle.Lock();
	auto& binary = podPtr->binary;

	if (binary.empty()) {
		if (!info.HasFlag("editor")) {
			LOG_WARN("Attempting to make shader stage {}: {} when compilation failed. keeping previous shader module.",
				GenMetaEnum(podPtr->stage).GetValueStr(), AssetRegistry::GetEntry(podHandle)->path);
		}
		lastCompileSuccess = false;
		return;
	}
	vk::ShaderModuleCreateInfo createInfo{};
	createInfo
		.setCodeSize(binary.size() * 4) //
		.setPCode(binary.data());
	module = Device->createShaderModuleUnique(createInfo);
	shaderStageCreateInfo.setStage(StageToVulkan(podPtr->stage)).setModule(*module).setPName("main");

	DEBUG_NAME(module, AssetRegistry::GetPodUri(podHandle));

	lastCompileSuccess = true;
}
