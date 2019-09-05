#pragma once

#include "assets/Asset.h"
#include "tinyxml2/tinyxml2.h"

class XMLDoc : public Asset
{
	tinyxml2::XMLDocument m_document;

public:
	XMLDoc(AssetManager* assetManager, const std::string& path)
		: Asset(assetManager, path) {}
	~XMLDoc() = default;

	bool Load(const std::string& path);
	void Clear() override;

	const tinyxml2::XMLElement* GetRootElement() const { return m_document.RootElement(); }

	void ToString(std::ostream& os) const override { os << "asset-type: XMLDocument, name: " << m_fileName; }
};
