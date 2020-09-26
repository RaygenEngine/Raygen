#pragma once
#include "rendering/assets/GpuAssetBase.h"

namespace vl {
struct GpuShaderStage : public GpuAssetTemplate<ShaderStage> {
	vk::UniqueShaderModule module;

	bool lastCompileSuccess{ false };

	vk::PipelineShaderStageCreateInfo shaderStageCreateInfo{};

	vk::ShaderStageFlagBits stage;

	GpuShaderStage(PodHandle<ShaderStage> podHandle);

	// Checks if the underlying VkShaderModule is set.
	// On failed compilations the shadermodule will not be reset and the previous one is preserved.
	// Use WasLastCompileSuccessful to detect the last compilation status
	[[nodiscard]] bool HasValidModule() const;

	// Checks if the last compile was successful.
	[[nodiscard]] bool WasLastCompileSuccessful() const;

	virtual void Update(const AssetUpdateInfo& info) override;
};
} // namespace vl
