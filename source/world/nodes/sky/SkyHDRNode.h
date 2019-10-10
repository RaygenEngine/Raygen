#pragma once

#include "world/nodes/Node.h"
#include "asset/loaders/ImageLoader.h"
#include "asset/pods/TexturePod.h"
#include "asset/PodHandle.h"

class SkyHDRNode : public Node {
	REFLECTED_NODE(SkyHDRNode, Node) { REFLECT_VAR(m_hdr); }

	PodHandle<TexturePod> m_hdr;

public:
	SkyHDRNode(Node* parent)
		: Node(parent)
	{
	}

	[[nodiscard]] PodHandle<TexturePod> GetSkyHDR() const { return m_hdr; }
};
