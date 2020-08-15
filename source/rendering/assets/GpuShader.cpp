#include "pch.h"
#include "GpuShader.h"

#include "assets/pods/Shader.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/Device.h"


using namespace vl;

GpuShader::GpuShader(PodHandle<Shader> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

bool GpuShader::HasValidModule() const
{
	return frag.Lock().HasValidModule() && vert.Lock().HasValidModule();
}

void GpuShader::Update(const AssetUpdateInfo& info)
{
	auto podPtr = podHandle.Lock();
	ClearDependencies();
	AddDependencies(podPtr->fragment, podPtr->vertex);

	frag = GpuAssetManager->GetGpuHandle(podPtr->fragment);
	vert = GpuAssetManager->GetGpuHandle(podPtr->vertex);

	if (frag.Lock().WasLastCompileSuccessful() && vert.Lock().WasLastCompileSuccessful()) {
		BuildShaderStages();
		if (onCompile) {
			onCompile();
		}
	}

	AddDependencies(podPtr->rayGen, podPtr->intersect, podPtr->anyHit, podPtr->closestHit, podPtr->miss);
	// WIP: Fix this
	// To Solve Shaders:
	// Use an array for all shader stages, use the enum of the stage as index
#define MAKE_SHADER(memVariable) memVariable = GpuAssetManager->GetGpuHandle(podPtr->##memVariable);


	MAKE_SHADER(rayGen);
	MAKE_SHADER(intersect);
	MAKE_SHADER(anyHit);
	MAKE_SHADER(closestHit);
	MAKE_SHADER(miss);
#undef MAKE_SHADER

	if (onCompileRayTracing) {
		onCompileRayTracing();
	}
}

void GpuShader::BuildShaderStages()
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
