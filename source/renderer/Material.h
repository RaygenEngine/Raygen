#pragma once


#include "asset/pods/MaterialPod.h"

#include "asset/AssetManager.h"

#include <vulkan/vulkan.hpp>


struct Material {

	std::unique_ptr<Texture> baseColorTexture;
	std::unique_ptr<Texture> metallicRoughnessTexture;
	std::unique_ptr<Texture> occlusionTexture;
	std::unique_ptr<Texture> normalTexture;
	std::unique_ptr<Texture> emissiveTexture;

	// WIP: (transfer each member individually?)
	PodHandle<MaterialPod> cpuData;

	Material(PodHandle<MaterialPod> podHandle);
};
