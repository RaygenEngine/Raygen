#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/ImageView.h"

struct IrradianceGrid_UBO {
};

#define IRRGRID_PROBE_COUNT 1024u

struct SceneIrradianceGrid : public SceneStruct {

	SceneIrradianceGrid();

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

	std::array<probe, IRRGRID_PROBE_COUNT> probes;

	BoolFlag shouldBuild{ true };

	float distToAdjacent{ 1.f };
	int32 ptSamples{ 2 };
	int32 ptBounces{ 2 };

	glm::vec4 pos;
};
