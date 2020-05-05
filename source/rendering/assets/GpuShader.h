#pragma once

#include "assets/AssetManager.h"
#include "assets/pods/Shader.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/resource/DescPoolAllocator.h"

#include <vulkan/vulkan.hpp>

struct Shader::Gpu : public vl::GpuAssetTemplate<Shader> {
	vk::UniqueShaderModule frag;
	vk::UniqueShaderModule vert;

	Shader::Gpu(PodHandle<Shader> podHandle);

	[[nodiscard]] bool HasCompiledSuccessfully() const;

	virtual void Update(const AssetUpdateInfo& info) override;

	std::function<void()> onCompile;


private:
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesCi;
	vl::DescriptorLayout descLayout;
	vk::UniquePipelineLayout pipelineLayout;

	std::vector<vk::PushConstantRange> pushConstantRanges;
};
