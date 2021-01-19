#pragma once
#include "rendering/wrappers/DescriptorSetLayout.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {

inline struct Layouts_ {

	inline constexpr static std::array gBufferColorAttachments = {
		std::pair{ "G_SNormal", vk::Format::eR16G16B16A16Snorm },
		std::pair{ "G_GNormal", vk::Format::eR16G16B16A16Snorm },
		std::pair{ "G_Albedo", vk::Format::eR32G32B32A32Sfloat },
		std::pair{ "G_SpecularColor", vk::Format::eR32G32B32A32Sfloat },
		std::pair{ "G_Emissive", vk::Format::eR8G8B8A8Srgb },
		std::pair{ "G_Velocity", vk::Format::eR32G32B32A32Sfloat },
		std::pair{ "G_UVDrawIndex", vk::Format::eR32G32B32A32Sfloat },
	};

	RDescriptorSetLayout gltfMaterialDescLayout;
	RDescriptorSetLayout singleUboDescLayout;
	RDescriptorSetLayout jointsDescLayout;
	RDescriptorSetLayout singleSamplerDescLayout;
	RDescriptorSetLayout singleSamplerFragOnlyLayout;
	RDescriptorSetLayout cubemapLayout;
	RDescriptorSetLayout envmapLayout;
	RDescriptorSetLayout accelLayout;

	RDescriptorSetLayout rtTriangleGeometry;

	RDescriptorSetLayout cubemapArray6;
	RDescriptorSetLayout cubemapArray64;
	RDescriptorSetLayout cubemapArray1024;
	RDescriptorSetLayout cubemapArray;
	RDescriptorSetLayout cubemapArrayStorage;

	RDescriptorSetLayout storageImageArray6;

	RDescriptorSetLayout singleStorageImage = GenerateStorageImageDescSet(1);
	RDescriptorSetLayout doubleStorageImage = GenerateStorageImageDescSet(2);
	RDescriptorSetLayout tripleStorageImage = GenerateStorageImageDescSet(3);
	RDescriptorSetLayout quadStorageImage = GenerateStorageImageDescSet(4);
	RDescriptorSetLayout tenStorageImage = GenerateStorageImageDescSet(10);
	RDescriptorSetLayout storageImageArray10;

	RDescriptorSetLayout singleStorageBuffer;

	RDescriptorSetLayout bufferAndSamplersDescLayout;


	RDescriptorSetLayout imageDebugDescLayout;

	// Global descriptor Set
	RDescriptorSetLayout globalDescLayout;

	RRenderPassLayout mainPassLayout;
	RRenderPassLayout secondaryPassLayout;
	RRenderPassLayout shadowPassLayout;
	RRenderPassLayout singleFloatColorAttPassLayout;
	RRenderPassLayout ptPassLayout;

	RRenderPassLayout svgfPassLayout;
	// Ray Trace Here
	RRenderPassLayout unlitPassLayout;
	// Output pass

	void MakeRenderPassLayouts();

	Layouts_();

private:
	static RDescriptorSetLayout GenerateStorageImageDescSet(size_t Count);
} * Layouts{};
} // namespace vl
