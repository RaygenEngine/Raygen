#pragma once

#include "world/nodes/Node.h"
#include "assets/texture/Texture.h"

class SkyHDRNode : public Node
{
	std::shared_ptr<Texture> m_hdrTexture;

public:
	SkyHDRNode(Node* parent);
	~SkyHDRNode() = default;
		
	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

	Texture* GetSkyHDR() const { return m_hdrTexture.get(); }

protected:
	std::string ToString(bool verbose, uint depth) const override;

public:

	void ToString(std::ostream& os) const override { os << "node-type: SkyHDRNode, name: " << GetName(); }
};
