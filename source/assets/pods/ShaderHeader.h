#pragma once
#include "assets/AssetPod.h"


struct ShaderHeader : public AssetPod {

	REFLECTED_POD(ShaderHeader)
	{
		REFLECT_ICON(FA_CODE);
		REFLECT_VAR(code, PropertyFlags::Multiline);
	}

	std::string code;
};
