#pragma once

#include "assets/Asset.h"
#include "tinyxml2/tinyxml2.h"

class XMLDoc : public Asset
{
	tinyxml2::XMLDocument m_document;

public:
	XMLDoc(const fs::path& path)
		: Asset(path) {}
	~XMLDoc() = default;

	const tinyxml2::XMLElement* GetRootElement() const { return m_document.RootElement(); }

protected:
	bool Load() override;
	void Unload() override;
};
