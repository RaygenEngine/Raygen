#pragma once

#include "core/reflection/GenMacros.h"
#include "asset/AssetPod.h"

#include "tinyxml2/tinyxml2.h"

struct XMLDocPod : public AssetPod
{
	REFLECTED_POD(XMLDocPod)
	{

	}
	static bool Load(XMLDocPod* pod, const uri::Uri& path);

	tinyxml2::XMLDocument document;
};

