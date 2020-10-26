#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/ImageView.h"

struct Reflprobe_UBO {
	glm::vec4 wipRemove;
};

struct CubemapMipFrames {
	std::array<vk::UniqueFramebuffer, 6> framebuffers;
	std::vector<vk::UniqueImageView> faceViews;
};

struct SceneReflprobe : public SceneStruct {

	SceneReflprobe();

	Reflprobe_UBO ubo{};

	vk::DescriptorSet reflDescSet;
	vk::DescriptorSet surroundingEnvStorageDescSet;
	vk::DescriptorSet surroundingEnvSamplerDescSet;

	vk::DescriptorSet ptcube_faceArrayDescSet;
	std::vector<vk::UniqueImageView> ptcube_faceViews;

	std::array<vk::UniqueFramebuffer, 6> irr_framebuffer;
	std::vector<vk::UniqueImageView> irr_faceViews;

	std::array<CubemapMipFrames, 6> pref_cubemapMips;

	vl::RCubemap surroundingEnv;
	vl::RCubemap irradiance;
	vl::RCubemap prefiltered;

	glm::vec4 position{};

	float innerRadius;
	float outerRadius;

	int32 ptSamples{ 16u };
	int32 ptBounces{ 3u };

	int32 resolution{ 128 };

	BoolFlag shouldBuild{ true };

	void ShouldResize(int32 resolution);
};
