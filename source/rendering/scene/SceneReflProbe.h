#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/ImageView.h"

struct Reflprobe_UBO {
	int32 lodCount{ 1 };
	float innerRadius{ 1.5f };
	float outerRadius{ 5.f };
	float irradianceFactor{ 1.f };
	glm::vec4 position{};
};

// WIP: this enity should only capture specular lods
struct SceneReflprobe : public SceneStruct {
	SceneReflprobe();
	void UploadUbo(uint32 curFrame) { UploadDataToUbo(curFrame, &ubo, sizeof(decltype(ubo))); }

	Reflprobe_UBO ubo{};

	vk::DescriptorSet environmentSamplerDescSet;
	vk::DescriptorSet environmentStorageDescSet;

	vk::DescriptorSet irradianceSamplerDescSet;
	vk::DescriptorSet irradianceStorageDescSet;

	// convolution based on roughness
	vk::DescriptorSet prefilteredSamplerDescSet;
	vk::DescriptorSet prefilteredStorageDescSet;

	vl::RCubemap environment;
	vl::RCubemap irradiance;
	vl::RCubemap prefiltered;

	std::vector<vk::UniqueImageView> prefilteredMipViews;

	int32 ptSamples{ 16 };
	int32 ptBounces{ 3 };

	int32 irrResolution{ 32 };

	int32 prefSamples{ 1024 };


	BoolFlag shouldBuild{ true };
	void Allocate();
};
