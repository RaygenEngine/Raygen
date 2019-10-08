#pragma once

#include "world/nodes/Node.h"
#include "asset/loaders/ImageLoader.h"
#include "asset/pods/ImagePod.h"
#include "asset/PodHandle.h"

class SkyHDRNode : public Node {
	REFLECTED_NODE(SkyHDRNode, Node) { REFLECT_VAR(m_hdrData); }

	// TODO: this is a texture
	PodHandle<ImagePod> m_hdrData;

public:
	SkyHDRNode(Node* parent);
	~SkyHDRNode() = default;

	PodHandle<ImagePod> GetSkyHDR() const { return m_hdrData; }
};
