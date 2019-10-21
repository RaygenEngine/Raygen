#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/camera/CameraNode.h"

#include <ovr/OVR_CAPI.h>

class OVRNode : public Node {
	REFLECTED_NODE(OVRNode, Node) {}


	ovrSession m_session;
	ovrHmdDesc m_hmd;
	ovrPosef m_eyePoses[2];

	size_t m_frameIndex{ 0 };

	CameraNode* m_eyes[2];

public:
	OVRNode();
	~OVRNode();

	void Update(float deltaTime) override;

	[[nodiscard]] ovrSession GetOVRSession() const { return m_session; }
	[[nodiscard]] const ovrHmdDesc& GetHMDDescription() const { return m_hmd; }
	[[nodiscard]] size_t GetFrameIndex() const { return m_frameIndex; }
	[[nodiscard]] CameraNode* GetEye(int32 index) const { return m_eyes[index]; }


	void IncrementFrameIndex() { ++m_frameIndex; }

	void EditLayerDetails(ovrLayerEyeFovDepth& ld);
};
