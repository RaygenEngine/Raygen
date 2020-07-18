#pragma once

#include "assets/pods/Shader.h"
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/resource/DescPoolAllocator.h"

#include <vulkan/vulkan.hpp>

namespace vl {
struct GpuShader : public vl::GpuAssetTemplate<Shader> {
	GpuShader(PodHandle<Shader> podHandle);

	vl::GpuHandle<ShaderStage> vert;
	vl::GpuHandle<ShaderStage> frag;

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;


	[[nodiscard]] bool HasValidModule() const;
	virtual void Update(const AssetUpdateInfo& info) override;
	std::function<void()> onCompile;

private:
	void BuildShaderStages();
};
} // namespace vl
