#include "pch.h"
#include "GpuShaderStage.h"

#include "rendering/Device.h"
#include "reflection/ReflEnum.h"

#include "assets/AssetRegistry.h"
using namespace vl;

GpuShaderStage::GpuShaderStage(PodHandle<ShaderStage> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

bool GpuShaderStage::HasValidModule() const
{
	return !(!module);
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
			LOG_ERROR("Attempting to make shader stage {} : {} with no binary data",
				GenMetaEnum(podPtr->stage).GetValueStr(), AssetHandlerManager::GetEntry(podHandle)->path);
		}
		lastCompileSuccess = false;
		return;
	}
	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(binary.size() * 4).setPCode(binary.data());
	module = vl::Device->createShaderModuleUnique(createInfo);
	lastCompileSuccess = true;
}
