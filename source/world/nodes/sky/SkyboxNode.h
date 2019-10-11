#pragma once

#include "world/nodes/Node.h"
#include "asset/loaders/CubemapLoader.h"

class SkyboxNode : public Node {
	REFLECTED_NODE(SkyboxNode, Node) { REFLECT_VAR(m_cubemap); }

	PodHandle<TexturePod> m_cubemap;

public:
	SkyboxNode(Node* parent)
		: Node(parent)
	{
	}

	[[nodiscard]] PodHandle<TexturePod> GetSkyMap() const { return m_cubemap; }
};
