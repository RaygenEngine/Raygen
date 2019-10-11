#pragma once

#include "asset/pods/XMLDocPod.h"
#include "asset/UriLibrary.h"

#include <tinyxml2/tinyxml2.h>

namespace XMLDocLoader {
inline bool Load(XMLDocPod* pod, const uri::Uri& path)
{
	return pod->document.LoadFile(uri::ToSystemPath(path)) == tinyxml2::XML_SUCCESS;
}
}; // namespace XMLDocLoader
