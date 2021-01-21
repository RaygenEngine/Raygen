#pragma once
#include "rendering/scene/SceneStructs.h"
#include "rendering/wrappers/ImageView.h"

struct Reflprobe_UBO {
	int32 lodCount{ 1 };
	float radius{ 1.5f };
	float irradianceFactor{ 1.f };
	float pad;
	glm::vec4 position{};
};

struct SceneReflprobe : public SceneStruct {
	SceneReflprobe();
	void UploadUbo(uint32 curFrame) { UploadDataToUbo(curFrame, &ubo, sizeof(Reflprobe_UBO)); }

	Reflprobe_UBO ubo{};

	vk::DescriptorSet environmentSamplerDescSet;
	vk::DescriptorSet irradianceSamplerDescSet;
	vk::DescriptorSet prefilteredSamplerDescSet;

	vl::RCubemap environment;
	vl::RCubemap irradiance;
	vl::RCubemap prefiltered;

	int32 ptSamples{ 16 };
	int32 ptBounces{ 3 };

	int32 irrResolution{ 32 };

	int32 prefSamples{ 1024 };


	BoolFlag shouldBuild{ true };
	void Allocate();
};
