#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/ImageView.h"

struct IrradianceGrid_UBO {
	int32 width{ 3 };
	int32 height{ 3 };
	int32 depth{ 3 };
	int32 builtCount{ 0 };

	glm::vec4 posAndDist;
};


struct SceneIrradianceGrid : public SceneStruct {
	SCENE_STRUCT(SceneIrradianceGrid);

	// WIP: this should be defined from the gridDescSet dynamic count
	static constexpr size_t gridProbeCount = 1024;
	IrradianceGrid_UBO ubo{};

	struct probe {
		vk::DescriptorSet surroundingEnvStorageDescSet;
		vk::DescriptorSet surroundingEnvSamplerDescSet;

		vk::DescriptorSet ptcube_faceArrayDescSet;
		std::vector<vk::UniqueImageView> ptcube_faceViews;

		std::vector<vk::UniqueFramebuffer> irr_framebuffer;
		std::vector<vk::UniqueImageView> irr_faceViews;

		vl::RCubemap surroundingEnv;
		vl::RCubemap irradiance;
	};

	vk::DescriptorSet gridDescSet;

	std::array<probe, gridProbeCount> probes;

	BoolFlag shouldBuild{ true };
	int32 ptSamples{ 2 };
	int32 ptBounces{ 2 };

	// PERF: this should only allocate if need be
	void Allocate();
};
