#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/ImageView.h"

struct Reflprobe_UBO {
	int32 lodCount;
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

	std::vector<CubemapMipFrames> pref_cubemapMips;

	vl::RCubemap surroundingEnv;
	vl::RCubemap irradiance;
	vl::RCubemap prefiltered;

	glm::vec4 position{};

	float innerRadius{ 1.5f };
	float outerRadius{ 70.f };

	int32 ptSamples{ 16 };
	int32 ptBounces{ 3 };

	BoolFlag shouldBuild{ true };

	void ShouldResize();
};
