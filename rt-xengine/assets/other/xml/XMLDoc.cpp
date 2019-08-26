#include "pch.h"

#include "assets/other/xml/XMLDoc.h"

namespace Assets
{
	XMLDoc::XMLDoc(DiskAssetManager* context, const std::string& path)
		: DiskAsset(context, path)
	{
	}

	bool XMLDoc::Load(const std::string& path)
	{
		return m_document.LoadFile(path.c_str()) == tinyxml2::XML_SUCCESS;
	}

	void XMLDoc::Clear()
	{
		m_document.Clear();
	}
}
