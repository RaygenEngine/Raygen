#pragma once

#include "assets/DiskAsset.h"
#include "assets/other/xml/ParsingAux.h"

namespace Assets
{
	class XMLDoc : public DiskAsset
	{
		tinyxml2::XMLDocument m_document;

	public:
		XMLDoc(DiskAssetManager* context, const std::string& path);
		~XMLDoc() = default;

		bool Load(const std::string& path);
		void Clear() override;

		const tinyxml2::XMLElement* GetRootElement() const { return m_document.RootElement(); }

		void ToString(std::ostream& os) const override { os << "asset-type: XMLDocument, name: " << m_name; }
	};
}
