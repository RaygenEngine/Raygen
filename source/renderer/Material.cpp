#include "pch/pch.h"

#include "renderer/LogicalDevice.h"

#include "renderer/VulkanLayer.h"

#include "renderer/Material.h"
#include "asset/AssetManager.h"


Material::Material(PodHandle<MaterialPod> podHandle)
{
	auto& device = VulkanLayer::device;

	// WIP:
	cpuData = podHandle;

	auto data = cpuData.Lock();

	baseColorTexture = std::make_unique<Texture>(data->baseColorTexture);
	metallicRoughnessTexture = std::make_unique<Texture>(data->metallicRoughnessTexture);
	occlusionTexture = std::make_unique<Texture>(data->occlusionTexture);
	normalTexture = std::make_unique<Texture>(data->normalTexture);
	emissiveTexture = std::make_unique<Texture>(data->emissiveTexture);
}
