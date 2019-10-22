#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/camera/CameraNode.h"

#include <ovr/OVR_CAPI.h>

// WIP
struct EyeCamera {
	int32 width, height;

	glm::mat4 projectionMatrix{};
	glm::mat4 viewMatrix{};
	glm::mat4 viewProjection{};

	glm::vec3 worldPos{};

	ovrFovPort fov{};
	ovrPosef pose{};
};

class OVRNode : public Node {
	REFLECTED_NODE(OVRNode, Node) { REFLECT_VAR(m_frameIndex, PropertyFlags::NoEdit); }

	ovrSession m_session;
	int64 m_frameIndex{ 0 };

	EyeCamera m_eyes[2];

	void RecalculateEyesMatrices();

public:
	OVRNode();
	~OVRNode();

	// WIP
	bool enabled{ false };
	bool visible{ false };

	void Update(float deltaTime) override;

	[[nodiscard]] ovrSession GetOVRSession() const { return m_session; }
	[[nodiscard]] size_t GetFrameIndex() const { return m_frameIndex; }
	[[nodiscard]] EyeCamera* GetEyeCamera(int32 index) { return &m_eyes[index]; }

	void IncrementFrameIndex() { ++m_frameIndex; }

	void EditLayerDetails(ovrLayerEyeFovDepth& ld) const;
	void EditLayerDetails(ovrLayerEyeFov& ld) const;

	void DirtyUpdate(DirtyFlagset flags) override;
};
