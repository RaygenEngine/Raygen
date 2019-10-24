#pragma once

#include "world/nodes/Node.h"
#include "asset/pods/TexturePod.h"

class SkyboxNode : public Node {
	REFLECTED_NODE(SkyboxNode, Node, DF_FLAGS(SkyTexture)) { REFLECT_VAR(m_cubemap).OnDirty(DF::SkyTexture); }

	PodHandle<TexturePod> m_cubemap;

public:
	[[nodiscard]] PodHandle<TexturePod> GetSkyMap() const { return m_cubemap; }
};
