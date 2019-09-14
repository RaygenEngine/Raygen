#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"

#include "tinyxml2/tinyxml2.h"

struct XMLDocPod : AssetPod
{
	tinyxml2::XMLDocument document;
};

