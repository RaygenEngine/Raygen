#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/ImageView.h"

struct Reflprobe_UBO {
	glm::vec4 position{};
	float innerRadius;
	float outerRadius;
};

struct CubemapMipFrames {
	std::array<vk::UniqueFramebuffer, 6> framebuffers;
	std::vector<vk::UniqueImageView> faceViews;
};

struct SceneReflprobe : SceneStruct {
	// SCENE_STRUCT(SceneReflprobe);

	SceneReflprobe();

	Reflprobe_UBO ubo;

	// UniquePtr<vl::AmbientBaker> ambientBaker;

	vk::DescriptorSet reflDescSet;
	vk::DescriptorSet surroundingEnvStorageDescSet;
	vk::DescriptorSet surroundingEnvSamplerDescSet;

	std::array<vk::DescriptorSet, 6> ptcube_faceDescSets;
	std::vector<vk::UniqueImageView> ptcube_faceViews;

	std::array<vk::UniqueFramebuffer, 6> irr_framebuffer;
	std::vector<vk::UniqueImageView> irr_faceViews;

	std::array<CubemapMipFrames, 6> pref_cubemapMips;

	vl::RCubemap surroundingEnv;
	vl::RCubemap irradiance;
	vl::RCubemap prefiltered;

	// WIP: store/load from asset
	// vl::GpuHandle<EnvironmentMap> envmap;

	BoolFlag shouldBuild{ true };

	void ShouldResize(int32 resolution);
};
