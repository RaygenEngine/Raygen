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
	void UploadUbo(uint32 curFrame);

	Irragrid_UBO ubo{};

	vk::DescriptorSet environmentSamplerDescSet;
	vk::DescriptorSet irradianceSamplerDescSet;

	vl::RCubemapArray environmentCubemaps;
	vl::RCubemapArray irradianceCubemaps;

	BoolFlag shouldBuild{ false };
	int32 ptSamples{ 2 };
	int32 ptBounces{ 2 };

	int32 irrResolution{ 32 };

	void Allocate();
};
