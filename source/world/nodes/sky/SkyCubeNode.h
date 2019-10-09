#pragma once

#include "world/nodes/Node.h"
#include "asset/loaders/CubemapLoader.h"

class SkyCubeNode : public Node {
	REFLECTED_NODE(SkyCubeNode, Node) { REFLECT_VAR(m_cubemap); }

	PodHandle<TexturePod> m_cubemap;

public:
	SkyCubeNode(Node* parent)
		: Node(parent)
	{
	}

	[[nodiscard]] PodHandle<TexturePod> GetSkyMap() const { return m_cubemap; }
};
