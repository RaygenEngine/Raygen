#include "pch.h"
#include "GpuShaderStage.h"

#include "rendering/Device.h"
#include "reflection/ReflEnum.h"

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

// void Shader::Gpu::GenerateLayouts(const Shader* pod)
//{
//	// Assume for now that both and only stages are frag & vert and they are valid
//
//	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
//	vertShaderStageInfo
//		.setStage(vk::ShaderStageFlagBits::eVertex) //
//		.setModule(*vert)
//		.setPName("main");
//
//	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
//	fragShaderStageInfo
//		.setStage(vk::ShaderStageFlagBits::eFragment) //
//		.setModule(*frag)
//		.setPName("main");
//
//	shaderStagesCi.emplace_back(vertShaderStageInfo);
//	shaderStagesCi.emplace_back(fragShaderStageInfo);
//
//	for (auto& desc : pod->vert.reflection.uboVariables) {
//		CLOG_ABORT(descLayout.bindings.size() != desc.binding, "Binding missmatch: {}", desc.name);
//
//		auto descType = desc.type == shd::VarType::Struct ? vk::DescriptorType::eUniformBuffer
//														  : vk::DescriptorType::eCombinedImageSampler;
//
//		descLayout.AddBinding(descType, vk::ShaderStageFlagBits::eFragment);
//	}
//
//	for (auto& desc : pod->frag.reflection.uboVariables) {
//		CLOG_ABORT(descLayout.bindings.size() != desc.binding, "Binding missmatch: {}", desc.name);
//
//		auto descType = desc.type == shd::VarType::Struct ? vk::DescriptorType::eUniformBuffer
//														  : vk::DescriptorType::eCombinedImageSampler;
//
//		descLayout.AddBinding(descType, vk::ShaderStageFlagBits::eVertex);
//	}
//	descLayout.Generate();
//}
