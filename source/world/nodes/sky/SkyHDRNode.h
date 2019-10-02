#pragma once

#include "world/nodes/Node.h"
#include "asset/loaders/ImageLoader.h"
#include "asset/pods/ImagePod.h"
#include "asset/PodHandle.h"

class SkyHDRNode : public Node
{
	REFLECTED_NODE(SkyHDRNode, Node) { }

	// TODO: this is a texture
	PodHandle<ImagePod> m_hdrData;
public:
	SkyHDRNode(Node* parent);
	~SkyHDRNode() = default;
		
	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

	PodHandle<ImagePod> GetSkyHDR() const { return m_hdrData; }

protected:
	std::string ToString(bool verbose, uint depth) const override;

public:

	void ToString(std::ostream& os) const override { os << "node-type: SkyHDRNode, name: " << GetName(); }
};
