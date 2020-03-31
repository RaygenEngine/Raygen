#pragma once

#include "assets/AssetManager.h"
#include "assets/pods/ShaderPod.h"
#include "rendering/asset/GpuAssetHandle.h"
#include "rendering/resource/DescPoolAllocator.h"

#include <vulkan/vulkan.hpp>

struct Shader::Gpu : public GpuAssetBase {
	vk::UniqueShaderModule frag;
	vk::UniqueShaderModule vert;

	Shader::Gpu(PodHandle<Shader> podHandle);

	[[nodiscard]] bool HasCompiledSuccessfully() const;


	void Z_Recompile();

	std::function<void()> onCompile;


private:
	const Shader* podPtr;
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesCi;
	R_DescriptorLayout descLayout;
	vk::UniquePipelineLayout pipelineLayout;

	std::vector<vk::PushConstantRange> pushConstantRanges;

	void GenerateLayouts(const Shader* pod);
};
