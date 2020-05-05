#include "pch.h"
#include "GpuShader.h"

#include "rendering/Device.h"

namespace {
void Compile(const Shader* pod, Shader::Gpu* gpu, std::string& outError)
{
	auto& fragBin = pod->fragment.Lock()->binary;
	auto& vertBin = pod->vertex.Lock()->binary;

	if (!(fragBin.size() && vertBin.size())) {
		outError = "Attempting to make shader with no binary data";
		return;
	}

	if (fragBin.size()) {
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.setCodeSize(fragBin.size() * 4).setPCode(fragBin.data());

		gpu->frag = vl::Device->createShaderModuleUnique(createInfo);
	}
	else {
		outError = "No fragment binary code found (failed to compile)";
	}

	if (vertBin.size()) {
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.setCodeSize(vertBin.size() * 4).setPCode(vertBin.data());

		gpu->vert = vl::Device->createShaderModuleUnique(createInfo);
	}
	else {
		outError = "No vertex binary code found (failed to compile)";
	}
}
} // namespace

Shader::Gpu::Gpu(PodHandle<Shader> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

bool Shader::Gpu::HasCompiledSuccessfully() const
{
	return frag && vert;
}

void Shader::Gpu::Update(const AssetUpdateInfo& info)
{
	auto podPtr = podHandle.Lock();
	ClearDependencies();
	AddDependencies(podPtr->fragment, podPtr->vertex);

	std::string err = "";
	Compile(podHandle.Lock(), this, err);


	if (!err.empty()) {
		if (!info.HasFlag("editor")) {
			LOG_ERROR("Gpu Shader Asset: {} Error: {}", AssetHandlerManager::GetEntry(podHandle)->path, err);
		}
	}
	else if (onCompile) {
		onCompile();
	}
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
