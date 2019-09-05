#include "pch.h"

#include "assets/other/xml/XMLDoc.h"

bool XMLDoc::Load(const std::string& path)
{
	return m_document.LoadFile(path.c_str()) == tinyxml2::XML_SUCCESS;
}

void XMLDoc::Clear()
{
	m_document.Clear();
}