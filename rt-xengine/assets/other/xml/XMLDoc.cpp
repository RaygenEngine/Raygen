#include "pch.h"

#include "assets/other/xml/XMLDoc.h"

bool XMLDoc::Load()
{
	return m_document.LoadFile(m_uri.string().c_str()) == tinyxml2::XML_SUCCESS;
}

void XMLDoc::Unload()
{
	m_document.Clear();
}
