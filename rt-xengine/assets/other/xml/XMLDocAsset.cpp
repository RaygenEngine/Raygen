#include "pch.h"

#include "assets/other/xml/XMLDocAsset.h"

bool XMLDocAsset::Load()
{
	return m_document.LoadFile(m_uri.string().c_str()) == tinyxml2::XML_SUCCESS;
}

void XMLDocAsset::Unload()
{
	m_document.Clear();
}
