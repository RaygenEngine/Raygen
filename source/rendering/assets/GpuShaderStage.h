#pragma once

#include "assets/pods/ShaderStage.h"
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/resource/DescPoolAllocator.h"

#include <vulkan/vulkan.hpp>

namespace vl {
struct GpuShaderStage : public GpuAssetTemplate<ShaderStage> {
	vk::UniqueShaderModule module;

	bool lastCompileSuccess{ false };

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
