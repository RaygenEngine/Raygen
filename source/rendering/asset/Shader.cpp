#include "pch.h"
#include "Shader.h"

#include "rendering/Device.h"

Shader::Gpu::Gpu(PodHandle<Shader> podHandle)
{
	auto pod = podHandle.Lock();


	if (pod->frag.binary.size()) {
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.setCodeSize(pod->frag.binary.size() * 4).setPCode(pod->frag.binary.data());

		frag = Device->createShaderModuleUnique(createInfo);
	}

	if (pod->vert.binary.size()) {
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.setCodeSize(pod->vert.binary.size() * 4).setPCode(pod->vert.binary.data());

		vert = Device->createShaderModuleUnique(createInfo);
	}

	// GenerateLayouts(pod);
}

bool Shader::Gpu::HasCompiledSuccessfully() const
{
	return frag && vert;
}

void Shader::Gpu::GenerateLayouts(const Shader* pod)
{
	// Assume for now that both and only stages are frag & vert and they are valid

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eVertex) //
		.setModule(*vert)
		.setPName("main");

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eFragment) //
		.setModule(*frag)
		.setPName("main");

	shaderStagesCi.emplace_back(vertShaderStageInfo);
	shaderStagesCi.emplace_back(fragShaderStageInfo);

	for (auto& desc : pod->vert.reflection.uboVariables) {
		CLOG_ABORT(descLayout.bindings.size() != desc.binding, "Binding missmatch: {}", desc.name);

		auto descType = desc.type == shd::VarType::Struct ? vk::DescriptorType::eUniformBuffer
														  : vk::DescriptorType::eCombinedImageSampler;

		descLayout.AddBinding(descType, vk::ShaderStageFlagBits::eFragment);
	}

	for (auto& desc : pod->frag.reflection.uboVariables) {
		CLOG_ABORT(descLayout.bindings.size() != desc.binding, "Binding missmatch: {}", desc.name);

		auto descType = desc.type == shd::VarType::Struct ? vk::DescriptorType::eUniformBuffer
														  : vk::DescriptorType::eCombinedImageSampler;

		descLayout.AddBinding(descType, vk::ShaderStageFlagBits::eVertex);
	}
	descLayout.Generate();
}
