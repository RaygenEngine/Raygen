#pragma once
#include "rendering/wrappers/DescriptorSetLayout.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {


// PERF: request from pool
inline struct PassLayouts_ {

	inline constexpr static std::array gBufferColorAttachments = {
		std::pair{ "G_SNormal", vk::Format::eR16G16B16A16Snorm },
		std::pair{ "G_GNormal", vk::Format::eR16G16B16A16Snorm },
		std::pair{ "G_Albedo", vk::Format::eR32G32B32A32Sfloat },
		std::pair{ "G_SpecularColor", vk::Format::eR32G32B32A32Sfloat },
		std::pair{ "G_Emissive", vk::Format::eR8G8B8A8Srgb },
		std::pair{ "G_Velocity", vk::Format::eR32G32B32A32Sfloat },
		std::pair{ "G_UVDrawIndex", vk::Format::eR32G32B32A32Sfloat },
	};

	// Render passes
	RRenderPassLayout main{ "Main Pass" };
	RRenderPassLayout secondary{ "Secondary Pass" };
	RRenderPassLayout shadow{ "Shadow Pass" };
	RRenderPassLayout singleFloatColorAtt{ "SingleFloatColor Pass" };
	RRenderPassLayout pt{ "PostProcess Pass" };
	RRenderPassLayout svgf{ "SVGF Pass" };
	RRenderPassLayout unlit{ "Unlit Pass" };

	PassLayouts_();

} * PassLayouts{};

// PERF: request from pool
inline struct DescriptorLayouts_ {

	// Uniform buffers
	RDescriptorSetLayout _1uniformBuffer = GenerateMultipleSameTypeDescLayout<1>(vk::DescriptorType::eUniformBuffer);

	// Storage buffers
	RDescriptorSetLayout _1storageBuffer = GenerateMultipleSameTypeDescLayout<1>(vk::DescriptorType::eStorageBuffer);


	// Sampler images
	RDescriptorSetLayout _1imageSampler
		= GenerateMultipleSameTypeDescLayout<1>(vk::DescriptorType::eCombinedImageSampler);

	// Storage images
	RDescriptorSetLayout _1storageImage = GenerateMultipleSameTypeDescLayout<1>(vk::DescriptorType::eStorageImage);
	RDescriptorSetLayout _2storageImage = GenerateMultipleSameTypeDescLayout<2>(vk::DescriptorType::eStorageImage);
	RDescriptorSetLayout _3storageImage = GenerateMultipleSameTypeDescLayout<3>(vk::DescriptorType::eStorageImage);
	RDescriptorSetLayout _4storageImage = GenerateMultipleSameTypeDescLayout<4>(vk::DescriptorType::eStorageImage);


	// Special descriptor layouts
	RDescriptorSetLayout global;
	RDescriptorSetLayout joints;
	RDescriptorSetLayout accelerationStructure;
	RDescriptorSetLayout _1storageBuffer_1024samplerImage;
	RDescriptorSetLayout _1imageSampler_2storageImage;
	RDescriptorSetLayout _1imageSamplerFragmentOnly;


	DescriptorLayouts_();

private:
	template<size_t Count>
	static RDescriptorSetLayout GenerateMultipleSameTypeDescLayout(vk::DescriptorType type)
	{
		using enum vk::ShaderStageFlagBits;
		using enum vk::DescriptorType;

		RDescriptorSetLayout descLayout;
		for (size_t i = 0; i < Count; i++) {
			descLayout.AddBinding(type, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
		}
		descLayout.Generate();
		return descLayout;
	}
} * DescriptorLayouts{};

} // namespace vl
