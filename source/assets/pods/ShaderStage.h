#pragma once
#include "assets/AssetPod.h"
#include "assets/shared/ShaderStageShared.h"

struct ShaderStage : public AssetPod {
	REFLECTED_POD(ShaderStage)
	{
		REFLECT_ICON(FA_CODE);

		REFLECT_VAR(stage);
		REFLECT_VAR(code, PropertyFlags::Multiline);
		REFLECT_VAR(headers /*, PropertyFlags::Advanced*/); // WIP: make this advanced
	}

	void PreprocessIncludes();
	std::vector<PodHandle<ShaderHeader>> headers;


	ShaderStageType stage{};

	SpirvReflection reflection;

	std::vector<uint32_t> binary;
	std::string code;
};
