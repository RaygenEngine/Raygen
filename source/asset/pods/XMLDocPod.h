#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"

#include "tinyxml2/tinyxml2.h"

struct XMLDocPod : DeletableAssetPod
{
	STATIC_REFLECTOR(XMLDocPod)
	{

	}
	static bool Load(XMLDocPod* pod, const fs::path& path);

	tinyxml2::XMLDocument document;
};

