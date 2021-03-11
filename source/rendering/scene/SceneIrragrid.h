#pragma once
#include "rendering/scene/SceneStructs.h"

struct Irragrid_UBO {
	int32 width{ 3 };
	int32 height{ 3 };
	int32 depth{ 3 };
	int32 builtCount{ 0 };

	XMFLOAT4A posAndDist;
};


struct SceneIrragrid : public SceneStruct {
	SceneIrragrid();
	Irragrid_UBO ubo{};

	BoolFlag shouldBuild{ false };
	int32 ptSamples{ 2 };
	int32 ptBounces{ 2 };

	int32 irrResolution{ 32 };
};
