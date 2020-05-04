#include "pch.h"
#include "GpuShader.h"

#include "rendering/Device.h"

namespace {
void Compile(const Shader* pod, Shader::Gpu* gpu)
{
	auto& fragBin = pod->fragment.Lock()->binary;
	auto& vertBin = pod->vertex.Lock()->binary;

	if (!(fragBin.size() && vertBin.size())) {
		return;
	}

	if (fragBin.size()) {
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.setCodeSize(fragBin.size() * 4).setPCode(fragBin.data());

		gpu->frag = vl::Device->createShaderModuleUnique(createInfo);
	}

	if (vertBin.size()) {
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.setCodeSize(vertBin.size() * 4).setPCode(vertBin.data());

		gpu->vert = vl::Device->createShaderModuleUnique(createInfo);
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

	Compile(podHandle.Lock(), this);
	if (HasCompiledSuccessfully() && onCompile) {
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
