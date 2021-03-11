#pragma once
#include "rendering/scene/SceneStructs.h"

struct Reflprobe_UBO {
	int32 lodCount{ 1 };
	float radius{ 1.5f };
	float irradianceFactor{ 1.f };
	float pad;
	XMFLOAT3A position{};
};

struct SceneReflprobe : public SceneStruct {
	SceneReflprobe();

	Reflprobe_UBO ubo{};


	int32 ptSamples{ 16 };
	int32 ptBounces{ 3 };

	int32 irrResolution{ 32 };

	int32 prefSamples{ 1024 };


	BoolFlag shouldBuild{ true };
};
