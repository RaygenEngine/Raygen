#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/ImageView.h"

struct IrradianceGrid_UBO {
};

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

		glm::vec4 pos;
	};

	vk::DescriptorSet gridDescSet;

	std::array<probe, 6> probes;

	BoolFlag shouldBuild{ true };

	float distToAdjacent{ 1.f };
	float blendProportion{ 0.2f };

	void ShouldRecalculatePositions(const glm::vec3& newPos);
};
