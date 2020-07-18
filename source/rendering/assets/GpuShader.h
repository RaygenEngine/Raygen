#pragma once
#include "rendering/assets/GpuAssetBase.h"

namespace vl {
struct GpuShader : public GpuAssetTemplate<Shader> {
	GpuShader(PodHandle<Shader> podHandle);

	GpuHandle<ShaderStage> vert;
	GpuHandle<ShaderStage> frag;

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;


	[[nodiscard]] bool HasValidModule() const;
	virtual void Update(const AssetUpdateInfo& info) override;
	std::function<void()> onCompile;

private:
	void BuildShaderStages();
};
} // namespace vl
