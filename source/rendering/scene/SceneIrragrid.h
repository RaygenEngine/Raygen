#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/ImageView.h"

struct Irragrid_UBO {
	int32 width{ 3 };
	int32 height{ 3 };
	int32 depth{ 3 };
	int32 builtCount{ 0 };

	glm::vec4 posAndDist;
};


struct SceneIrragrid : public SceneStruct {
	SceneIrragrid();
	void UploadUbo(uint32 curFrame) { UploadDataToUbo(curFrame, &ubo, sizeof(decltype(ubo))); }

	Irragrid_UBO ubo{};

	vk::DescriptorSet environmentSamplerDescSet;
	vk::DescriptorSet environmentStorageDescSet;

	vk::DescriptorSet irradianceSamplerDescSet;
	vk::DescriptorSet irradianceStorageDescSet;

	vl::RCubemapArray environmentCubemaps;
	vl::RCubemapArray irradianceCubemaps;

	BoolFlag shouldBuild{ true };
	int32 ptSamples{ 2 };
	int32 ptBounces{ 2 };

	int32 irrResolution{ 32 };

	void Allocate();
};
