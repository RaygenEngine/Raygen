#pragma once

#include "asset/pods/XMLDocPod.h"

#include "tinyxml2/tinyxml2.h"

namespace XMLDocLoader
{
	inline bool Load(XMLDocPod* pod, const fs::path& path)
	{
		return pod->document.LoadFile(path.string().c_str()) == tinyxml2::XML_SUCCESS;
	}
};
