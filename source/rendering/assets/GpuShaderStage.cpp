#include "pch.h"
#include "GpuShaderStage.h"

#include "assets/AssetRegistry.h"
#include "assets/pods/ShaderStage.h"
#include "reflection/ReflEnum.h"
#include "rendering/Device.h"
#include "assets/util/SpirvCompiler.h"

using namespace vl;

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
	stage = shd::StageToVulkan(podPtr->stage);

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
	shaderStageCreateInfo.setStage(stage).setModule(*module).setPName("main");

	DEBUG_NAME(module, AssetRegistry::GetPodUri(podHandle));

	lastCompileSuccess = true;
}
