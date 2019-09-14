#include "pch.h"

#include "asset/assets/XMLDocAsset.h"

bool XMLDocAsset::Load()
{
	return m_pod->document.LoadFile(m_uri.string().c_str()) == tinyxml2::XML_SUCCESS;
}
