#include "pch.h"

#include "asset/assets/XMLDocAsset.h"

bool XMLDocAsset::Load(XMLDocPod* pod, const fs::path& path)
{
	return pod->document.LoadFile(path.string().c_str()) == tinyxml2::XML_SUCCESS;
}
