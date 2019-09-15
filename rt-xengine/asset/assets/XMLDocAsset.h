#pragma once

#include "asset/Asset.h"
#include "asset/pods/XMLDocPod.h"

#include "tinyxml2/tinyxml2.h"

struct XMLDocAsset
{
	static bool Load(XMLDocPod* pod, const fs::path& path);
};
