#ifndef XMLDOC_H
#define XMLDOC_H

#include "assets/DiskAsset.h"

#include "tinyxml2/tinyxml2.h"
#include <string>

namespace Assets
{
	class XMLDoc : public DiskAsset
	{
		tinyxml2::XMLDocument m_document;

	public:
		XMLDoc(DiskAssetManager* context);
		~XMLDoc() = default;
	
		bool Load(const std::string& path);
		void Clear() override;

		const tinyxml2::XMLElement* GetRootElement() const { return m_document.RootElement(); }

		void ToString(std::ostream& os) const override { os << "asset-type: XMLDocument, name: " << m_label; }
	};
}

#endif // XMLDOC_H
