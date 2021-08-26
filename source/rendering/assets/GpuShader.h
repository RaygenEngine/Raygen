#pragma once
#include "rendering/assets/GpuAssetBase.h"

#include "rendering/VkCoreIncludes.h"

namespace vl {
struct GpuShader : public GpuAssetTemplate<Shader> {
	GpuShader(PodHandle<Shader> podHandle);

	GpuHandle<ShaderStage> vert;
	GpuHandle<ShaderStage> frag;


	GpuHandle<ShaderStage> rayGen;
	GpuHandle<ShaderStage> intersect;
	GpuHandle<ShaderStage> anyHit;
	GpuHandle<ShaderStage> closestHit;
	GpuHandle<ShaderStage> miss;

	GpuHandle<ShaderStage> callable;

	GpuHandle<ShaderStage> compute;

	// ONLY includes vert & frag. DOES NOT include raytracing stages. Just use the handles directly for ray tracing.
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;


	[[nodiscard]] bool HasValidModule() const;
	virtual void Update(const AssetUpdateInfo& info) override;
	std::function<void()> onCompile;


	std::function<void()> onCompileRayTracing;

private:
	void BuildShaderStages();
};
} // namespace vl
