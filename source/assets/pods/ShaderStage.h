#pragma once
#include "assets/AssetPod.h"
#include "assets/shared/ShaderStageShared.h"

struct ShaderStage : public AssetPod {
	REFLECTED_POD(ShaderStage)
	{
		REFLECT_ICON(FA_CODE);

		REFLECT_VAR(stage);
		REFLECT_VAR(code, PropertyFlags::Multiline);
	}

	ShaderStageType stage{};

	SpirvReflection reflection;

	std::vector<uint32_t> binary;
	std::string code;
};
