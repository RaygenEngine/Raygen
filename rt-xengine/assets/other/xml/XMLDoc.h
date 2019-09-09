#pragma once

#include "assets/FileAsset.h"
#include "tinyxml2/tinyxml2.h"

class XMLDoc : public FileAsset
{
	tinyxml2::XMLDocument m_document;

public:
	XMLDoc(const std::string& path)
		: FileAsset(path) {}
	~XMLDoc() = default;

	bool Load(const std::string& path);
	void Clear() override;

	const tinyxml2::XMLElement* GetRootElement() const { return m_document.RootElement(); }

	void ToString(std::ostream& os) const override { os << "asset-type: XMLDocument, name: " << m_fileName; }
};
