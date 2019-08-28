#include "pch.h"

#include "assets/other/xml/XMLDoc.h"

namespace Assets
{
	XMLDoc::XMLDoc(EngineObject* pObject, const std::string& path)
		: DiskAsset(pObject, path)
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
