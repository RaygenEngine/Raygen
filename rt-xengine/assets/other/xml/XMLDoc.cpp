#include "pch.h"
#include "XMLDoc.h"

namespace Assets
{
	XMLDoc::XMLDoc(DiskAssetManager* context)
		: DiskAsset(context)
	{
	}

	bool XMLDoc::Load(const std::string& path)
	{
		SetIdentificationFromPath(path);

		return m_document.LoadFile(path.c_str()) == tinyxml2::XML_SUCCESS;
	}

	void XMLDoc::Clear()
	{
		m_document.Clear();
	}
}
