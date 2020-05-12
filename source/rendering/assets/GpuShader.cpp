#include "pch.h"
#include "GpuShader.h"

#include "rendering/Device.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/assets/GpuAssetManager.h"

Shader::Gpu::Gpu(PodHandle<Shader> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

bool Shader::Gpu::HasValidModule() const
{
	return frag.Lock().HasValidModule() && vert.Lock().HasValidModule();
}

void Shader::Gpu::Update(const AssetUpdateInfo& info)
{
	auto podPtr = podHandle.Lock();
	ClearDependencies();
	AddDependencies(podPtr->fragment, podPtr->vertex);

	frag = vl::GpuAssetManager->GetGpuHandle(podPtr->fragment);
	vert = vl::GpuAssetManager->GetGpuHandle(podPtr->vertex);

	if (frag.Lock().WasLastCompileSuccessful() && vert.Lock().WasLastCompileSuccessful()) {
		BuildShaderStages();
		if (onCompile) {
			onCompile();
		}
	}
}

void Shader::Gpu::BuildShaderStages()
{
	shaderStages.clear();
	
	// shaders
	auto& vertShaderModule = *vert.Lock().module;
	auto& fragShaderModule = *frag.Lock().module;

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eVertex) //
		.setModule(vertShaderModule)
		.setPName("main");

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eFragment) //
		.setModule(fragShaderModule)
		.setPName("main");

	shaderStages.push_back(vertShaderStageInfo);
	shaderStages.push_back(fragShaderStageInfo);
}
