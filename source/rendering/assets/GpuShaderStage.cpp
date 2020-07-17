#include "pch.h"
#include "GpuShaderStage.h"

#include "rendering/Device.h"
#include "reflection/ReflEnum.h"

#include "assets/AssetRegistry.h"

ShaderStage::Gpu::Gpu(PodHandle<ShaderStage> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

bool ShaderStage::Gpu::HasValidModule() const
{
	return !(!module);
}

bool ShaderStage::Gpu::WasLastCompileSuccessful() const
{
	return lastCompileSuccess;
}

void ShaderStage::Gpu::Update(const AssetUpdateInfo& info)
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
